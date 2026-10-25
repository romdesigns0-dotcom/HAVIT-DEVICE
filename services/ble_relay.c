#include "havit_ble_relay.h"
#include "havit_device_claim.h"
#include "havit_tasks.h"
#include "havit_connectivity.h"
#include "protocomm.h"
#include "protocomm_ble.h"
#include "protocomm_security.h"
#include "protocomm_security1.h"
#include "cJSON.h"
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "mbedtls/gcm.h"
#include "mbedtls/base64.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define RELAY_CAP 300000
#define CHUNK 160
static protocomm_t *s_pc;
static SemaphoreHandle_t s_lock;
static char *s_tx,*s_rx;
static char *s_reset_proof;
static size_t s_reset_proof_len;
static int64_t s_reset_proof_deadline;
static uint32_t s_reset_owner_session;
static size_t s_tx_len,s_rx_len,s_rx_at;
static int64_t s_deadline;
static char s_nonce[65],s_binding[40];
static uint32_t s_owner_session;
static const char *str(cJSON *j,const char *k){cJSON *v=cJSON_GetObjectItemCaseSensitive(j,k);return cJSON_IsString(v)?v->valuestring:"";}
static void clear(void){heap_caps_free(s_tx);heap_caps_free(s_rx);s_tx=s_rx=NULL;s_tx_len=s_rx_len=s_rx_at=0;s_deadline=0;s_nonce[0]=0;s_binding[0]=0;}
static void hex(const uint8_t *in,size_t n,char *out){for(size_t i=0;i<n;i++)snprintf(out+i*2,3,"%02x",in[i]);}
static bool unhex(const char *in,uint8_t *out,size_t n){if(strlen(in)!=n*2)return false;for(size_t i=0;i<n;i++){unsigned v;if(sscanf(in+i*2,"%2x",&v)!=1)return false;for(int j=0;j<2;j++){char c=in[i*2+j];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;}out[i]=v;}return true;}
static char *seal(cJSON *body,havit_device_claim_snapshot_t *claim){
    char *plain=cJSON_PrintUnformatted(body);if(!plain)return NULL;
    size_t len=strlen(plain),encoded_len=0;uint8_t key[32],iv[12],tag[16],nonce[32];
    uint8_t *cipher=heap_caps_malloc(len+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    char *encoded=heap_caps_malloc(4*((len+2)/3)+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if(!cipher||!encoded||havit_device_relay_key(key)!=ESP_OK){heap_caps_free(cipher);heap_caps_free(encoded);cJSON_free(plain);return NULL;}
    esp_fill_random(iv,sizeof(iv));esp_fill_random(nonce,sizeof(nonce));hex(nonce,sizeof(nonce),s_nonce);
    snprintf(s_binding,sizeof(s_binding),"%s",claim->binding_id);
    char aad[200];snprintf(aad,sizeof(aad),"havit-relay-v1|request|%s|%s|%s",claim->device_id,s_binding,s_nonce);
    mbedtls_gcm_context ctx;mbedtls_gcm_init(&ctx);
    int err=mbedtls_gcm_setkey(&ctx,MBEDTLS_CIPHER_ID_AES,key,256);
    if(!err)err=mbedtls_gcm_crypt_and_tag(&ctx,MBEDTLS_GCM_ENCRYPT,len,iv,sizeof(iv),(uint8_t*)aad,strlen(aad),(uint8_t*)plain,cipher,sizeof(tag),tag);
    mbedtls_gcm_free(&ctx);memset(key,0,sizeof(key));cJSON_free(plain);
    if(!err)err=mbedtls_base64_encode((uint8_t*)encoded,4*((len+2)/3)+1,&encoded_len,cipher,len);
    heap_caps_free(cipher);if(err){heap_caps_free(encoded);return NULL;}encoded[encoded_len]=0;
    char iv_hex[25],tag_hex[33];hex(iv,sizeof(iv),iv_hex);hex(tag,sizeof(tag),tag_hex);
    cJSON *e=cJSON_CreateObject();cJSON_AddNumberToObject(e,"protocol",1);cJSON_AddStringToObject(e,"deviceId",claim->device_id);
    cJSON_AddStringToObject(e,"bindingId",s_binding);cJSON_AddStringToObject(e,"nonce",s_nonce);
    cJSON_AddStringToObject(e,"iv",iv_hex);cJSON_AddStringToObject(e,"tag",tag_hex);cJSON_AddStringToObject(e,"data",encoded);
    heap_caps_free(encoded);char *result=cJSON_PrintUnformatted(e);cJSON_Delete(e);return result;
}
static bool prepare(void){
    clear();havit_device_claim_snapshot_t claim;havit_device_claim_snapshot(&claim);if(!claim.binding_id[0]||!claim.identity_provisioned)return false;
    const char query[]="{\"protocol\":1,\"op\":\"snapshot\"}";uint8_t *raw=NULL;ssize_t n=0;
    if(havit_tasks_ble_endpoint(0,(uint8_t*)query,strlen(query),&raw,&n,NULL)!=ESP_OK)return false;
    cJSON *local=cJSON_ParseWithLength((char*)raw,n);free(raw);
    cJSON *pending=cJSON_GetObjectItemCaseSensitive(local,"pending");if(!cJSON_IsArray(pending)){cJSON_Delete(local);return false;}
    cJSON *body=cJSON_CreateObject();cJSON_AddNumberToObject(body,"protocol",1);cJSON_AddItemToObject(body,"pending",cJSON_Duplicate(pending,true));
    s_tx=seal(body,&claim);cJSON_Delete(body);cJSON_Delete(local);
    if(!s_tx)return false;
    s_tx_len=strlen(s_tx);s_deadline=esp_timer_get_time()+300000000LL;return s_tx_len<RELAY_CAP;
}
static int apply_reply(char **reset_proof){
    havit_device_claim_snapshot_t claim;havit_device_claim_snapshot(&claim);
    cJSON *e=cJSON_ParseWithLength(s_rx,s_rx_len);uint8_t key[32],iv[12],tag[16];
    bool valid=e&&cJSON_GetObjectItemCaseSensitive(e,"protocol")&&cJSON_GetObjectItemCaseSensitive(e,"protocol")->valueint==1&&
      strcmp(str(e,"nonce"),s_nonce)==0&&strcmp(str(e,"deviceId"),claim.device_id)==0&&
      strcmp(str(e,"bindingId"),claim.binding_id)==0&&strcmp(s_binding,claim.binding_id)==0&&
      unhex(str(e,"iv"),iv,sizeof(iv))&&unhex(str(e,"tag"),tag,sizeof(tag))&&havit_device_relay_key(key)==ESP_OK;
    if(!valid){cJSON_Delete(e);return 0;}
    const char *encoded=str(e,"data");size_t cap=strlen(encoded)*3/4+4,n=0;
    uint8_t *cipher=heap_caps_malloc(cap,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT),*plain=heap_caps_malloc(cap+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    int err=(!cipher||!plain)?-1:mbedtls_base64_decode(cipher,cap,&n,(uint8_t*)encoded,strlen(encoded));
    char aad[200];snprintf(aad,sizeof(aad),"havit-relay-v1|response|%s|%s|%s",claim.device_id,s_binding,s_nonce);
    mbedtls_gcm_context ctx;mbedtls_gcm_init(&ctx);
    if(!err)err=mbedtls_gcm_setkey(&ctx,MBEDTLS_CIPHER_ID_AES,key,256);
    if(!err)err=mbedtls_gcm_auth_decrypt(&ctx,n,iv,sizeof(iv),(uint8_t*)aad,strlen(aad),tag,sizeof(tag),cipher,plain);
    mbedtls_gcm_free(&ctx);memset(key,0,sizeof(key));heap_caps_free(cipher);cJSON_Delete(e);
    if(err){heap_caps_free(plain);return 0;}plain[n]=0;
    cJSON *body=cJSON_ParseWithLength((char*)plain,n);heap_caps_free(plain);if(!body)return 0;
    cJSON *factory=cJSON_GetObjectItemCaseSensitive(body,"factoryReset");
    if(cJSON_IsObject(factory)){
        const char *id=str(factory,"requestId");
        bool shaped=strlen(id)==36;
        for(int i=0;shaped&&i<36;i++)if((i==8||i==13||i==18||i==23)?id[i]!='-':!((id[i]>='0'&&id[i]<='9')||(id[i]>='a'&&id[i]<='f')))shaped=false;
        cJSON *ack=cJSON_CreateObject();
        if(!shaped||!ack){cJSON_Delete(ack);cJSON_Delete(body);return 0;}
        cJSON_AddNumberToObject(ack,"protocol",1);cJSON_AddStringToObject(ack,"factoryResetAck",id);
        char *sealed=seal(ack,&claim);cJSON_Delete(ack);
        esp_err_t queued=sealed?havit_connectivity_factory_reset():ESP_ERR_INVALID_ARG;
        cJSON_Delete(body);
        if(queued!=ESP_OK){cJSON_free(sealed);return 0;}
        *reset_proof=sealed;return 2;
    }
    cJSON_AddStringToObject(body,"op","apply_snapshot");char *request=cJSON_PrintUnformatted(body);cJSON_Delete(body);
    uint8_t *result=NULL;ssize_t result_len=0;
    err=request?havit_tasks_ble_endpoint(0,(uint8_t*)request,strlen(request),&result,&result_len,NULL):ESP_ERR_NO_MEM;cJSON_free(request);
    cJSON *response=err==ESP_OK?cJSON_ParseWithLength((char*)result,result_len):NULL;free(result);
    bool applied=cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(response,"applied"));cJSON_Delete(response);
    if(applied && havit_device_claim_confirm_bridge(s_binding)) (void)havit_connectivity_bridge_confirmed();
    return applied?1:0;
}
static uint32_t u32(const uint8_t *b){return (uint32_t)b[0]|((uint32_t)b[1]<<8)|((uint32_t)b[2]<<16)|((uint32_t)b[3]<<24);}
static esp_err_t endpoint(uint32_t session,const uint8_t *in,ssize_t len,uint8_t **out,ssize_t *out_len,void *ctx){
    (void)ctx;*out=malloc(CHUNK+2048);if(!*out)return ESP_ERR_NO_MEM;(*out)[0]=1;*out_len=1;
    if(!in||len<1||len>CHUNK+5)return ESP_OK;
    xSemaphoreTake(s_lock,portMAX_DELAY);
    if(s_deadline&&esp_timer_get_time()>s_deadline)clear();
    uint32_t at=len>=5?u32(in+1):0;
    if(in[0]==1&&len==1){
        if(s_deadline&&session!=s_owner_session)goto done;
        if(!s_tx){if(!prepare())goto done;s_owner_session=session;}
        for(int i=0;i<4;i++)(*out)[i+1]=(s_tx_len>>(i*8))&255;
        *out_len=5;(*out)[0]=0;
    }else if(in[0]==6&&len==1&&s_reset_proof&&s_reset_owner_session==session&&esp_timer_get_time()<s_reset_proof_deadline){
        for(int i=0;i<4;i++)(*out)[i+1]=(s_reset_proof_len>>(i*8))&255;
        *out_len=5;(*out)[0]=0;
    }else if(in[0]==7&&len==5&&s_reset_proof&&s_reset_owner_session==session&&esp_timer_get_time()<s_reset_proof_deadline&&at<s_reset_proof_len){
        size_t n=s_reset_proof_len-at;if(n>CHUNK)n=CHUNK;memcpy(*out+1,s_reset_proof+at,n);*out_len=n+1;(*out)[0]=0;
    }else if(!s_deadline||s_owner_session!=session)goto done;
    else if(in[0]==2&&len==5&&at<s_tx_len){size_t n=s_tx_len-at;if(n>CHUNK)n=CHUNK;memcpy(*out+1,s_tx+at,n);*out_len=n+1;(*out)[0]=0;}
    else if(in[0]==3&&len==5&&at>0&&at<RELAY_CAP){heap_caps_free(s_rx);s_rx=heap_caps_malloc(at+1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);s_rx_len=at;s_rx_at=0;if(s_rx)(*out)[0]=0;}
    else if(in[0]==4&&len>5&&s_rx&&at==s_rx_at&&s_rx_at+len-5<=s_rx_len){memcpy(s_rx+s_rx_at,in+5,len-5);s_rx_at+=len-5;(*out)[0]=0;}
    else if(in[0]==5&&len==1&&s_rx&&s_rx_at==s_rx_len){
        s_rx[s_rx_len]=0;char *proof=NULL;int applied=apply_reply(&proof);
        if(applied){(*out)[0]=(uint8_t)applied;if(applied==2&&proof){cJSON_free(s_reset_proof);s_reset_proof=proof;s_reset_proof_len=strlen(proof);s_reset_proof_deadline=esp_timer_get_time()+30000000LL;s_reset_owner_session=session;}}
        clear();
    }
done:xSemaphoreGive(s_lock);return ESP_OK;
}
bool havit_ble_relay_active(void){return s_pc!=NULL;}
void havit_ble_relay_disconnected(void){if(s_lock){xSemaphoreTake(s_lock,portMAX_DELAY);clear();cJSON_free(s_reset_proof);s_reset_proof=NULL;s_reset_proof_len=0;s_reset_proof_deadline=0;s_reset_owner_session=0;xSemaphoreGive(s_lock);}}
void havit_ble_relay_stop(void){if(s_pc){protocomm_ble_stop(s_pc);protocomm_delete(s_pc);s_pc=NULL;}havit_ble_relay_disconnected();}
esp_err_t havit_ble_relay_start(void){
    if(s_pc)return ESP_OK;
    if(!s_lock)s_lock=xSemaphoreCreateMutex();
    if(!s_lock)return ESP_ERR_NO_MEM;
    havit_device_claim_snapshot_t claim;havit_device_claim_snapshot(&claim);if(!claim.binding_id[0])return ESP_ERR_INVALID_STATE;
    s_pc=protocomm_new();if(!s_pc)return ESP_ERR_NO_MEM;
    static protocomm_ble_name_uuid_t lookup[]={{"prov-session",0xff51},{"proto-ver",0xff53},{"havit-relay",0xff54}};
    protocomm_ble_config_t config={.service_uuid={0x5d,0xcc,0xef,0x92,0x11,0x8a,0x42,0xb1,0x99,0x0a,0x4c,0x72,0xa3,0x88,0x91,0x67},.nu_lookup=lookup,.nu_lookup_count=3};
    snprintf(config.device_name,sizeof(config.device_name),"HAVIT-SYNC-%.4s",strlen(claim.name)>6?claim.name+6:claim.name);
    esp_err_t err=protocomm_ble_start(s_pc,&config);
    if(err==ESP_OK)err=protocomm_set_security(s_pc,"prov-session",&protocomm_security1,NULL);
    if(err==ESP_OK)err=protocomm_set_version(s_pc,"proto-ver","{\"prov\":{\"ver\":\"v1.1\",\"sec_ver\":1,\"cap\":[\"no_pop\"]},\"havit_relay\":1}");
    if(err==ESP_OK)err=protocomm_add_endpoint(s_pc,"havit-relay",endpoint,NULL);
    if(err!=ESP_OK)havit_ble_relay_stop();
    return err;
}

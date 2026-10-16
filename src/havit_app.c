/**
 * @file havit_app.c
 */

#include "havit_app.h"

#include <string.h>

#define HAVIT_STORE_KEY_ONBOARDING_COMPLETE "onboarding_complete"

static void copy_display_name(char destination[HAVIT_USER_NAME_MAX], const char *source)
{
    if(source == NULL) {
        destination[0] = '\0';
        return;
    }

    strncpy(destination, source, HAVIT_USER_NAME_MAX - 1);
    destination[HAVIT_USER_NAME_MAX - 1] = '\0';
}

havit_app_result_t havit_app_init(havit_app_t *app, havit_store_t store)
{
    bool onboarding_complete = false;

    if(app == NULL || store.read_bool == NULL || store.write_bool == NULL) {
        return HAVIT_APP_INVALID_ARGUMENT;
    }

    memset(app, 0, sizeof(*app));
    app->store = store;

    /* A missing key is normal on first boot. The storage adapter reports false. */
    (void)app->store.read_bool(app->store.context,
                               HAVIT_STORE_KEY_ONBOARDING_COMPLETE,
                               &onboarding_complete);

    app->state.onboarding_complete = onboarding_complete;
    app->state.screen = onboarding_complete ? HAVIT_SCREEN_SIGN_IN : HAVIT_SCREEN_ONBOARDING;
    app->state.auth_state = HAVIT_AUTH_IDLE;
    return HAVIT_APP_OK;
}

havit_app_result_t havit_app_dispatch(havit_app_t *app,
                                     havit_event_t event,
                                     const char *display_name)
{
    if(app == NULL || app->store.write_bool == NULL) {
        return HAVIT_APP_INVALID_ARGUMENT;
    }

    switch(event) {
        case HAVIT_EVENT_CONTINUE:
            if(app->state.screen == HAVIT_SCREEN_ONBOARDING) {
                if(!app->store.write_bool(app->store.context,
                                          HAVIT_STORE_KEY_ONBOARDING_COMPLETE,
                                          true)) {
                    return HAVIT_APP_STORAGE_ERROR;
                }
                app->state.onboarding_complete = true;
                app->state.screen = HAVIT_SCREEN_SIGN_IN;
                app->state.auth_state = HAVIT_AUTH_IDLE;
                return HAVIT_APP_OK;
            }
            if(app->state.screen == HAVIT_SCREEN_WELCOME_BACK) {
                app->state.screen = HAVIT_SCREEN_HOME;
                return HAVIT_APP_OK;
            }
            return HAVIT_APP_INVALID_TRANSITION;

        case HAVIT_EVENT_LOGIN_STARTED:
            if(app->state.screen != HAVIT_SCREEN_SIGN_IN) {
                return HAVIT_APP_INVALID_TRANSITION;
            }
            app->state.auth_state = HAVIT_AUTH_PENDING;
            return HAVIT_APP_OK;

        case HAVIT_EVENT_LOGIN_SUCCEEDED:
            if(app->state.screen != HAVIT_SCREEN_SIGN_IN ||
               app->state.auth_state != HAVIT_AUTH_PENDING ||
               display_name == NULL || display_name[0] == '\0') {
                return HAVIT_APP_INVALID_TRANSITION;
            }
            copy_display_name(app->state.display_name, display_name);
            app->state.auth_state = HAVIT_AUTH_AUTHENTICATED;
            app->state.screen = HAVIT_SCREEN_WELCOME_BACK;
            return HAVIT_APP_OK;

        case HAVIT_EVENT_LOGIN_FAILED:
            if(app->state.screen != HAVIT_SCREEN_SIGN_IN ||
               app->state.auth_state != HAVIT_AUTH_PENDING) {
                return HAVIT_APP_INVALID_TRANSITION;
            }
            app->state.auth_state = HAVIT_AUTH_FAILED;
            return HAVIT_APP_OK;

        case HAVIT_EVENT_LOGOUT:
            if(app->state.screen != HAVIT_SCREEN_HOME &&
               app->state.screen != HAVIT_SCREEN_WELCOME_BACK) {
                return HAVIT_APP_INVALID_TRANSITION;
            }
            app->state.auth_state = HAVIT_AUTH_IDLE;
            app->state.display_name[0] = '\0';
            app->state.screen = HAVIT_SCREEN_SIGN_IN;
            return HAVIT_APP_OK;

        case HAVIT_EVENT_BACK:
            if(app->state.screen == HAVIT_SCREEN_SIGN_IN) {
                app->state.screen = HAVIT_SCREEN_ONBOARDING;
                return HAVIT_APP_OK;
            }
            if(app->state.screen == HAVIT_SCREEN_WELCOME_BACK) {
                app->state.screen = HAVIT_SCREEN_SIGN_IN;
                app->state.auth_state = HAVIT_AUTH_IDLE;
                app->state.display_name[0] = '\0';
                return HAVIT_APP_OK;
            }
            return HAVIT_APP_INVALID_TRANSITION;

        default:
            return HAVIT_APP_INVALID_ARGUMENT;
    }
}

const havit_app_state_t *havit_app_get_state(const havit_app_t *app)
{
    return app == NULL ? NULL : &app->state;
}

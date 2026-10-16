/**
 * @file havit_app.h
 * Platform-independent application state and navigation for Havit OS.
 */

#ifndef HAVIT_APP_H
#define HAVIT_APP_H

#include "havit_store.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAVIT_USER_NAME_MAX 48

typedef enum {
    HAVIT_SCREEN_ONBOARDING = 0,
    HAVIT_SCREEN_SIGN_IN,
    HAVIT_SCREEN_WELCOME_BACK,
    HAVIT_SCREEN_HOME /* UI screen is not implemented yet. */
} havit_screen_t;

typedef enum {
    HAVIT_AUTH_IDLE = 0,
    HAVIT_AUTH_PENDING,
    HAVIT_AUTH_AUTHENTICATED,
    HAVIT_AUTH_FAILED
} havit_auth_state_t;

typedef enum {
    HAVIT_EVENT_CONTINUE = 0,
    HAVIT_EVENT_LOGIN_STARTED,
    HAVIT_EVENT_LOGIN_SUCCEEDED,
    HAVIT_EVENT_LOGIN_FAILED,
    HAVIT_EVENT_LOGOUT,
    HAVIT_EVENT_BACK
} havit_event_t;

typedef enum {
    HAVIT_APP_OK = 0,
    HAVIT_APP_INVALID_ARGUMENT,
    HAVIT_APP_INVALID_TRANSITION,
    HAVIT_APP_STORAGE_ERROR
} havit_app_result_t;

typedef struct {
    havit_screen_t screen;
    havit_auth_state_t auth_state;
    bool onboarding_complete;
    char display_name[HAVIT_USER_NAME_MAX];
} havit_app_state_t;

typedef struct {
    havit_store_t store;
    havit_app_state_t state;
} havit_app_t;

/** Initialize state from storage. Missing onboarding data means first boot. */
havit_app_result_t havit_app_init(havit_app_t *app, havit_store_t store);

/** Apply a user or authentication event and update the application state. */
havit_app_result_t havit_app_dispatch(havit_app_t *app,
                                     havit_event_t event,
                                     const char *display_name);

/** Read the current application state. */
const havit_app_state_t *havit_app_get_state(const havit_app_t *app);

#ifdef __cplusplus
}
#endif

#endif /* HAVIT_APP_H */

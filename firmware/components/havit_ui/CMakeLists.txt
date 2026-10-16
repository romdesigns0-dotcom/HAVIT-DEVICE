idf_component_register(
    SRCS
        "../../../ui/components/havit_default_background.c"
        "../../../ui/assets/login2_google_g.c"
        "../../../ui/assets/login1_continue_arrow.c"
        "../../../ui/modules/blank_screen.c"
        "../../../ui/modules/eyes_intro.c"
        "../../../ui/modules/login1.c"
        "../../../ui/modules/login_flow.c"
        "../../../ui/animations/eyes_intro_animation.c"
        "../../../ui/fonts/instrument_sans_regular_14.c"
        "../../../ui/fonts/instrument_sans_regular_22.c"
        "../../../ui/fonts/instrument_sans_regular_15.c"
        "../../../ui/fonts/instrument_sans_regular_40.c"
    INCLUDE_DIRS "../../../ui/include"
    REQUIRES havit_hardware lvgl)

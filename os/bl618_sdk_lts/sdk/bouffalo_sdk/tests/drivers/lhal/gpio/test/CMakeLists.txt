﻿sdk_generate_library()
sdk_add_include_directories(.)

sdk_library_add_sources(test_gpio_input_high.c)
sdk_library_add_sources(test_gpio_input_low.c)

sdk_library_add_sources(test_runners/test_gpio_input_runner.c)
sdk_library_add_sources(test_runners/test_gpio_interrupt_runner.c)
sdk_library_add_sources(test_runners/test_gpio_output_runner.c)
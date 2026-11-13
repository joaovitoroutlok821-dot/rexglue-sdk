test_aot_runtime_localized_gpr:
  #_ REGISTER_IN r3 0x0000000000000029

  addi r11, r3, 7
  addi r3, r11, 0
  blr

  #_ REGISTER_OUT r3 0x0000000000000030

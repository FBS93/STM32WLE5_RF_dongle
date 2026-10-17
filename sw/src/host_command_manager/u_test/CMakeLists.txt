ecf_create_etf_test(
  host_command_reception
  host_command_reception/src/host_command_reception.c
  LINKS host_command_manager eff emf eaf
)

ecf_create_etf_test(
  host_command_set_config
  host_command_set_config/src/host_command_set_config.c
  LINKS host_command_manager eff emf eaf
)

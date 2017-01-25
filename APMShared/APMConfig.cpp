#include "APMConfig.h"
#include <stdio.h>

APMConfig::APMConfig() {
	//defaults
    // по умолчанию запуск программы осуществляется по хоткею
	trigger_method		= TRIGGER_BY_HOTKEY;
	trigger_process		= NULL;
    // запись в лог файл не ведется
//	log_file			= NULL;
    log_file			= "apmmeter.log";
    // и еще что то типа пропустить начало
	skip_begin			= TRUE;

//	if(argc > 1)
//		for(int i=1;i<argc;i++) {
//            if(strcmp(argv[i],"-o") == 0 && (i+1) < argc) {
//				log_file = argv[i+1];
//            } else if(strcmp(argv[i],"-p") == 0 && (i+1) < argc) {
//				trigger_method = TRIGGER_BY_PROCESS;
//				trigger_process = argv[i+1];
//            } else if(strcmp(argv[i],"--no-skip-begin") == 0) {
//				skip_begin = FALSE;
//            } else if(strcmp(argv[i],"--help") == 0) {
//				showHelp();
//				exit(0);
//			}
//		}
}

void APMConfig::showHelp() {
//    printf("implement me ^^");
}

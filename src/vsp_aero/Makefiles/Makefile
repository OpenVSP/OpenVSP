include Makefile.COMMON

################################################################################      
#                                                                              #
#                              COMPILE SCRIPT                                  #
#                                                                              #
################################################################################

help:
	@echo '                                                  '
	@echo '   *** VSPAERO System Descrition File ***         '
	@echo '                                                  '
	@echo '           Code Installation                      ' 
	@echo '                                                  '
	@echo '                                                  '
	@echo ' Syntax: make [Machine_Type]                      '
	@echo '                                                  '
	@echo '                                                  '
	@echo ' Supported Machine Types:                         '
	@echo '                                                  '
	@echo '   linux     .......... intel linux               '
	@echo '                                                  '
	@echo ' Examples:                                        '
	@echo '                                                  '
	@echo ' make linux  .... updates VSPAERO on an linux     '
	@echo ' make clean ... removes .o files                  '
	@echo ' make help .... displays this message             '

linux:
	@echo '     '
	@echo 'Updating VSPAERO for linux on intel                '
	@echo '     '
	@make -f Makefile.linux
	@echo '     '
	@echo '     '
	@echo '     '

linux-clang:
	@echo '     '
	@echo 'Updating VSPAERO for linux on intel                '
	@echo '     '
	@make -f Makefile.linux.clang
	@echo '     '
	@echo '     '
	@echo '     '	

################################################################################      
#                                                                              #
#                              CLEAN THINGS UP                                 #
#                                                                              #
################################################################################

dist	:
	@echo '      '
	@echo 'Cleaning up models for distribution...'
	@cd ../models/cone            ; rm -rf *.dat *.adb *.fem *.plt *.lod *.slc *.str cbaero.exe.stackdump core
	
clean   :
	@echo '     '
	@echo ' *** Removing executables, object files... '
	@echo '     '
	@cd $(VSPAERO_SOLVER_DIR)       	     ; rm -rf *.o
	@cd $(VSPAERO_ADJOINT_DIR)       	  ; rm -rf *.o
	@cd $(VIEWER_DIR)       	     		  ; rm -rf *.o
	
	
	
	
	
	
	
	
	
	
	

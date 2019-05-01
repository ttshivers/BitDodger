################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/bin/cl430" -vmsp --use_hw_mpy=none --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/Users/dsm66/workspace_v8/final" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.4.LTS/include" --advice:power="all" --define=__MSP430G2553__ -g --c99 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --enum_type=packed --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '



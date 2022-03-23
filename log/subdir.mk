################################################################################
#
# Sub-directory make file.
#
# Note:
#     Exported from Eclipse using automatic make file generation.
#
################################################################################


#
#-------------------------------------------------------------------------------
# As much as possible, to avoid modifying this file when changes of the
# following nature occur:
#     * addition or removal of source files
#     * this sub-directory moves to another location
# automatically determine the directory (relative to the project's base
# directory) this make file lives in.
#-------------------------------------------------------------------------------
#

#
# The make command appends the name of each make file to the MAKEFILE_LIST
# variable as it processes make files.  MAKEFILE_LIST is a "list" of
# space-delimited path names and this file's relative path is the last
# "element" in that "list".
#

makefile_path := $(lastword $(MAKEFILE_LIST))

#
# Now that we have this file's relative path, we need the directory part of
# that path name
#

makefile_directory := $(dir $(makefile_path))

#
# Remove the trailing '/' from the directory
#

makefile_directory := $(patsubst %/, %, $(makefile_directory))


#
#-------------------------------------------------------------------------------
# As much as possible, to avoid modifying this file when changes of the
# following nature occur:
#     * addition or removal of source files
#     * this sub-directory moves to another location
# automatically determine the source files to compile then generate the
# corresponding object/depend file names.
#-------------------------------------------------------------------------------
#

#
# Add any source files here which should not be compiled
#

ignore_files :=

#
# Automatically generate source file names in this directory
#

source_files := $(wildcard $(makefile_directory)/*.c)

#
# Filter out any source files that should not be compiled
#

source_files := $(filter-out $(ignore_files), $(source_files))


# Add inputs and outputs from these tool invocations to the build variables 

C_SRCS += $(source_files)

OBJS += $(addsuffix .o, $(basename $(source_files)))

C_DEPS += $(addsuffix .d, $(basename $(source_files)))


# Each subdirectory must supply rules for building sources it contributes
$(makefile_directory)/%.o: $(makefile_directory)/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	$(CC) $(CFLAGS) $(MACRO_DEFS) $(INCLUDE_DIRS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

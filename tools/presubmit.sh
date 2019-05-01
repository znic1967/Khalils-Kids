#!/bin/bash

STATUS_CAPTURE=0
BUILD_CAPTURE=0
LINT_CAPTURE=0
TIDY_CAPTURE=0
TEST_CAPTURE=0

# Get base path
SJBASE=$(dirname "$0")
SJBASE=$(cd "$SJBASE/.." ; pwd -P)

function print_divider
{
    printf "\e[1;33m======================================================= "
    printf "\e[0m\n\n"
}

function print_status
{
    if [ $1 -ne 0 ]
    then
        printf "\e[31m✘\e[0;31m"
    else
        printf "\e[32m✔\e[0;31m"
    fi
}

function check
{
    if [ $1 -ne 0 ]
        then
        lint="$(print_status $LINT_CAPTURE)"
        commit="$(print_status $STATUS_CAPTURE)"
        test="$(print_status $TEST_CAPTURE)"
        tidy="$(print_status $TIDY_CAPTURE)"
        build="$(print_status $BUILD_CAPTURE)"
        printf "\e[0;31m ================================ \e[0m\n"
        printf "\e[1;31m|        None of this!           |\e[0m\n"
        printf "\e[1;31m|                                |\e[0m\n"
        printf "\e[1;31m|        (╯°□ °)╯︵ ┻━┻          |\e[0m\n"
        printf "\e[1;31m|                                |\e[0m\n"
        printf "\e[1;31m|      Don't even PUSH!          |\e[0m\n"
        printf "\e[0;31m ================================ \e[0m\n"
        printf "\e[0;31m|                                |\e[0m\n"
        printf "\e[0;31m| Code style must be lint free %b |\e[0m\n" $lint
        printf "\e[0;31m|    Code style must be tidy %b   |\e[0m\n" $tidy
        printf "\e[0;31m|     Commit must be clean %b     |\e[0m\n" $commit
        printf "\e[0;31m|       Tests must pass %b        |\e[0m\n" $test
        printf "\e[0;31m|       Code must build %b        |\e[0m\n" $build
        printf "\e[0;31m|                                |\e[0m\n"
        printf "\e[0;31m ================================ \e[0m\n"
        exit 1
    else
        printf "\e[0;32m ============================ \e[0m\n"
        printf "\e[0;32m| Everything looks good here |\e[0m\n"
        printf "\e[0;32m|                            |\e[0m\n"
        printf "\e[0;32m|           (•_•)            |\e[0m\r"
        sleep .5
        printf "\e[0;32m|           ( •_•)>⌐▪-▪      |\e[0m\r"
        sleep .5
        printf "\e[0;32m|           (⌐▪_▪)           |\e[0m\n"
        sleep .5
        printf "\e[0;32m|                            |\e[0m\n"
        printf "\e[0;32m|      You may commit        |\e[0m\n"
        printf "\e[0;32m ============================ \e[0m\n"
        exit 0
    fi
}
####################################
#    Clean Git Repository Check    #
####################################
printf "\e[0;33mChecking if git repo is clean\e[0m "
git diff-index --quiet HEAD --
STATUS_CAPTURE=$?
print_status $STATUS_CAPTURE
echo ""
####################################
#    All Projects Build Check      #
####################################
print_divider
printf "Checking that all projects build\n\n"

printf "\e[0;33mBuilding HelloWorld Project\e[0m "
# Change to the HelloWorld project
cd "$SJBASE/firmware/HelloWorld"
# Clean the build and start building from scratch
make -s clean
# Check if the system can build without any warnings!
SILENCE=$(make -s application WARNINGS_ARE_ERRORS=-Werror)
# Set build capture to return code from the build
BUILD_CAPTURE=$?
print_status $BUILD_CAPTURE
echo ""

printf "\e[0;33mBuilding Hyperload Bootloader\e[0m "
# Change to the Hyperload project
cd "$SJBASE/firmware/Hyperload"
# Clean the build and start building from scratch
make -s clean
# Check if the system can build without any warnings!
SILENCE=$(make -s bootloader WARNINGS_ARE_ERRORS=-Werror)
# Set build capture to return code from the build
SPECIFIC_BUILD_CAPTURE=$?
BUILD_CAPTURE=$(($BUILD_CAPTURE + $SPECIFIC_BUILD_CAPTURE))
print_status $SPECIFIC_BUILD_CAPTURE
echo ""

# Build all example projects
cd $SJBASE/firmware/examples
for d in */; do

cd "$SJBASE/firmware/examples/$d"

printf "\e[0;33mBuilding Example $d\e[0m "
# Clean the build and start building from scratch
make -s clean
# Check if the system can build without any warnings!
SILENCE=$(make -s application WARNINGS_ARE_ERRORS=-Werror)
# Add the return codes of the previous build capture. None zero means that at
# least one of the captures failed.
SPECIFIC_BUILD_CAPTURE=$?
BUILD_CAPTURE=$(($BUILD_CAPTURE + $SPECIFIC_BUILD_CAPTURE))
print_status $SPECIFIC_BUILD_CAPTURE
echo ""

done

# Return to home project
cd $SJBASE/firmware/HelloWorld
####################################
#           Lint Check             #
####################################
print_divider

printf "\e[0;33mExecuting 'lint' check \e[0m"
make -s lint 1> /dev/null
LINT_CAPTURE=$?
print_status $LINT_CAPTURE
echo ""
####################################
#         Clang Tidy Check         #
####################################
print_divider

printf "\e[0;33mExecuting 'tidy' check \e[0m\n"
make -s tidy
TIDY_CAPTURE=$?
print_status $TIDY_CAPTURE
echo ""
####################################
#         Unit Test Check          #
####################################
print_divider

printf "\e[0;33mBuilding and running unit tests \e[0m\n"
make -s test WARNINGS_ARE_ERRORS=-Werror
TEST_BUILD_CAPTURE=$?
make -s run-test
TEST_RUN_CAPTURE=$?
TEST_CAPTURE=$(($TEST_BUILD_CAPTURE + $TEST_RUN_CAPTURE))
print_status $TEST_CAPTURE
echo ""

# Check if there were any errors. For this to succeed, this value should be 0
check $(($STATUS_CAPTURE+$BUILD_CAPTURE+$LINT_CAPTURE+$TIDY_CAPTURE+$TEST_CAPTURE))

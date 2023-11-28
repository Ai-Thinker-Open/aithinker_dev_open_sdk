#!/bin/bash

set -o pipefail

# Define colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

# set default value
CMD=$0
CWD="$(dirname $(readlink -f $0))"
BASE_DIR="${CWD}/../.."
OUTPUT_FILE="${CWD}/.compile_results.json"
ERR_FILE="${CWD}/.compile_results.err"
SKIP_FILE="${CWD}/.compile_results.skip"
SUCCESS_FILE="${CWD}/.compile_results.success"
build_fails=true
gen_result=true
TARGET_APP=""

function usage() {
	echo -e "${GREEN}Usage: $CMD -f <input-file> [-o <output-file>] [-c <change-number> -p <patchset-number>] [-e] [-t <app-name>] ${NC}"
	echo -e "${GREEN}Options:${NC}"
	echo -e "${GREEN}  -c <change-number> Gerrit change number.${NC}"
	echo -e "${GREEN}  -e Do not build failed app when file \"compile_err.json\" exist.${NC}"
	echo -e "${GREEN}  -f <input-file> Input json file.${NC}"
	echo -e "${GREEN}  -h Print this help message.${NC}"
	echo -e "${GREEN}  -o <output-file> Output json file.${NC}"
	echo -e "${GREEN}  -r Disable print build result.${NC}"
	echo -e "${GREEN}  -t <app-name> One of the app name.${NC}"
	echo -e "${GREEN}  -p <patchset-number>: Gerrit patchset number.${NC}"
}

# Parse command line options
while getopts "c:ef:ho:rt:p:" opt; do
	case ${opt} in
	c)
		GERRIT_CHANGE_NUMBER=${OPTARG}
		;;
	e)
		build_fails=false
		;;
	f)
		# INPUT_FILE=${OPTARG}
		INPUT_FILE=$(readlink -f "${OPTARG}")
		;;
	o)
		OUTPUT_FILE=$(readlink -f "${OPTARG}")
		;;
	r)
		gen_result=false
		;;
	t)
		TARGET_APP=${OPTARG}
		build_fails=false
		;;
	p)
		GERRIT_PATCHSET_NUMBER=${OPTARG}
		;;
	h)
		usage
		exit 1
		;;
	\?)
		usage
		exit 1
		;;
	:)
		usage
		echo "Option -$OPTARG requires an argument."
		exit 1
		;;
	esac
done

INPUT_FILE_NAME="$(basename $INPUT_FILE)"
OUTPUT_FILE_NAME="$(basename $OUTPUT_FILE)"
OUTPUT_FILE_DIR="$(dirname $OUTPUT_FILE)"
EXCEL_FILE="${OUTPUT_FILE_DIR}/${OUTPUT_FILE_NAME%.*}.xlsx"
CSV_FILE="${OUTPUT_FILE_DIR}/${OUTPUT_FILE_NAME%.*}.csv"
ERR_FILE="${OUTPUT_FILE_DIR}/${OUTPUT_FILE_NAME%.*}.err"
SKIP_FILE="${OUTPUT_FILE_DIR}/${OUTPUT_FILE_NAME%.*}.skip"
SUCCESS_FILE="${OUTPUT_FILE_DIR}/${OUTPUT_FILE_NAME%.*}.success"

# debug echo
# echo "CWD:          $CWD"
# echo "BASE_DIR      $BASE_DIR"
# echo "INPUT_FILE    $INPUT_FILE"
# echo "OUTPUT_FILE   $OUTPUT_FILE"
# echo "OUTPUT_DIR    $OUTPUT_FILE_DIR"
# echo "OUTPUT_NAME   $OUTPUT_FILE_NAME"
# echo "EXCEL_FILE    $EXCEL_FILE"
# echo "CSV_FILE      $CSV_FILE"
# echo "ERR_FILE      $ERR_FILE"
# echo "BUILD_FAIL"   $build_fails

function arg_check() {
	# Check if a json file is specified
	if [ -z "$INPUT_FILE" ]; then
		usage
		exit 1
	fi

	# Use ERR_FILE instead of INPUT_FILE
	if [[ "$build_fails" == "true" ]] && [ -f "$ERR_FILE" ]; then
		echo -e "${YELLOW}Find err file in $ERR_FILE,${NC}"
		echo -e "${YELLOW}use it instead of $INPUT_FILE${NC}"
		INPUT_FILE=$ERR_FILE
	fi
}

function tools_check() {
	# Check if the command repo exists
	if ! command -v repo &>/dev/null; then
		echo -e "${RED}Failed: Command repo not found${NC}"
		echo -e "${RED}Install: curl https://mirrors.tuna.tsinghua.edu.cn/git/git-repo -o repo && chmod +x repo${NC}"
		exit 1
	fi

	# Check if the command jq exists
	if ! command -v jq &>/dev/null; then
		echo -e "${RED}Failed: Command jq not found${NC}"
		echo -e "${RED}Install: apt-get install jq${NC}"
		exit 1
	fi

	# Check if the command sponge exists
	if ! command -v sponge &>/dev/null; then
		echo -e "${RED}Failed: Command sponge not found${NC}"
		echo -e "${RED}Install: apt-get install moreutils${NC}"
		exit 1
	fi

	# Check if the command column exists
	if ! command -v column &>/dev/null; then
		echo -e "${RED}Failed: Command column not found${NC}"
		echo -e "${RED}Install: apt-get install bsdmainutils${NC}"
		exit 1
	fi

	# Check if the command sort exists
	if ! command -v sort &>/dev/null; then
		echo -e "${RED}Failed: Command sort not found${NC}"
		echo -e "${RED}Install: apt-get install coreutils${NC}"
		exit 1
	fi
}

function del_artifacts() {
	# remove build result
	if [ -f $OUTPUT_FILE ] || [ -f $EXCEL_FILE ] || [ -f $CSV_FILE ] || [ -f $SKIP_FILE ] || [ -f $SUCCESS_FILE ]; then
		echo -e "${YELLOW}Delete build artifacts${NC}"
		rm -f $OUTPUT_FILE $EXCEL_FILE $CSV_FILE $SKIP_FILE $SUCCESS_FILE
	fi

	if [[ "$build_fails" == "false" ]] && [ -f "$ERR_FILE" ]; then
		echo -e "${YELLOW}Delete $ERR_FILE${NC}"
		rm $ERR_FILE
	fi

	cp $INPUT_FILE $OUTPUT_FILE
}

function gerrit_review() {
	local change_number=$1
	local patchset_number=$2
	local msg=$3

	if [ -n "$change_number" ] && [ -n "$patchset_number" ]; then
		nohup ssh -n -p 29418 jenkins@gerrit.bouffalolab.com gerrit review ${change_number},${patchset_number} --message \'$msg\' >/dev/null 2>&1 &
	fi
}

function build_target_app() {
	local enable=$(jq -r ".apps[] | select(.name == \"$TARGET_APP\") | .enable" $INPUT_FILE)
	local name=$(jq -r ".apps[] | select(.name == \"$TARGET_APP\") | .name" $INPUT_FILE)
	local cmd=$(jq -r ".apps[] | select(.name == \"$TARGET_APP\") | .cmd" $INPUT_FILE)
	local path=$(jq -r ".apps[] | select(.name == \"$TARGET_APP\") | .path" $INPUT_FILE)

	if [ "$enable" = "false" ]; then
		# Add compile result to skipped
		jq --arg path "$path" '.apps |= map(if .path == $path then . + {result: "skipped"} else . end)' $OUTPUT_FILE | sponge $OUTPUT_FILE
		continue
	fi

	echo -en "\n${YELLOW}Making $name:$path...${NC}\n"
	cd "$BASE_DIR/$path"

	# Check if build dir exists
	if [ -d build ]; then
		rm -rf build
	fi

	# Check if CMakeLists.txt exists in the path directory
	if [ ! -f CMakeLists.txt ]; then
		echo -e "${RED}Skiped: CMakeLists.txt not found in $path${NC}"
		continue
	fi

	# Check if the command to build the project exists
	if ! command -v $cmd &>/dev/null; then
		echo -e "${RED}Skipped: Command '$cmd' not found${NC}"
		continue
	fi

	# Run the command to build the project
	echo -e "${YELLOW}Exec command: $cmd${NC}"
	eval $cmd
	ret=$?

	# Check if the build succeeded
	if [ $ret -eq 0 ]; then
		echo -e "${GREEN}Build $name succeeded from $path${NC}"

		# Add compile result to success
		jq --arg path "$path" '.apps |= map(if .path == $path then . + {result: "success"} else . end)' $OUTPUT_FILE | sponge $OUTPUT_FILE
	else
		echo -e "${RED}Build $name failed from $path${NC}"

		# Add compile result to failed
		jq --arg path "$path" '.apps |= map(if .path == $path then . + {result: "failed"} else . end)' $OUTPUT_FILE | sponge $OUTPUT_FILE

		# Post failed event to gerrit
		# if [ -n "$GERRIT_CHANGE_NUMBER" ] && [ -n "$GERRIT_PATCHSET_NUMBER" ]; then
		# 	message="Error: \"${cmd}\" failed in \"$path\" directory."
		# 	gerrit_review "$GERRIT_CHANGE_NUMBER" "$GERRIT_PATCHSET_NUMBER" "$message"
		# fi
	fi

	# remove build directory
	if [ -d build ]; then
		rm -rf build
	fi

	cd $BASE_DIR

	return
}

function build_all_apps() {
	# Loop through each app in the json file
	jq -c '.apps[]' $INPUT_FILE | while read app; do
		local enable=$(echo $app | jq -r '.enable')
		local name=$(echo $app | jq -r '.name')
		local cmd=$(echo $app | jq -r '.cmd')
		local path=$(echo $app | jq -r '.path')

		if [ "$enable" = "false" ]; then
			# Add compile result to skipped
			jq --arg path "$path" '.apps |= map(if .path == $path then . + {result: "skipped"} else . end)' $OUTPUT_FILE | sponge $OUTPUT_FILE
			continue
		fi

		if [[ "$build_fails" == "true" ]] && [ -f "$ERR_FILE" ]; then
			result=$(echo $app | jq -r '.result')
			if [[ "$result" == "success" ]]; then
				continue
			fi
		fi

		echo -en "\n${YELLOW}Making $name:$path...${NC}\n"
		cd "$BASE_DIR/$path"

		# Check if build dir exists
		if [ -d build ]; then
			rm -rf build
		fi

		# Check if CMakeLists.txt exists in the path directory
		if [ ! -f CMakeLists.txt ]; then
			echo -e "${RED}Skiped: CMakeLists.txt not found in $path${NC}"
			continue
		fi

		# Check if the command to build the project exists
		if ! command -v $cmd &>/dev/null; then
			echo -e "${RED}Skipped: Command '$cmd' not found${NC}"
			continue
		fi

		# Run the command to build the project
		echo -e "${YELLOW}Exec command: $cmd${NC}"
		eval $cmd
		ret=$?

		# Check if the build succeeded
		if [ $ret -eq 0 ]; then
			echo -e "${GREEN}Build $name succeeded from $path${NC}"

			# Add compile result to success
			jq --arg path "$path" '.apps |= map(if .path == $path then . + {result: "success"} else . end)' $OUTPUT_FILE | sponge $OUTPUT_FILE
		else
			echo -e "${RED}Build $name failed from $path${NC}"

			# Add compile result to failed
			jq --arg path "$path" '.apps |= map(if .path == $path then . + {result: "failed"} else . end)' $OUTPUT_FILE | sponge $OUTPUT_FILE

			# Post failed event to gerrit
			# if [ -n "$GERRIT_CHANGE_NUMBER" ] && [ -n "$GERRIT_PATCHSET_NUMBER" ]; then
			# 	message="Error: \"${cmd}\" failed in \"$path\" directory."
			# 	gerrit_review "$GERRIT_CHANGE_NUMBER" "$GERRIT_PATCHSET_NUMBER" "$message"
			# fi
		fi

		# remove build directory
		if [ -d build ]; then
			rm -rf build
		fi

		cd $BASE_DIR
	done
	echo -en "\n${GREEN}All apps build finished!${NC}\n"
}

function get_results() {
	if [ ! -f $OUTPUT_FILE ]; then
		echo -e "${RED}Failed: File $OUTPUT_FILE not found${NC}"
		exit 1
	fi

	if [ -n "$TARGET_APP" ]; then
		jq '.apps[] |= if has("result") then . else . + {"result": "skipped"} end' $OUTPUT_FILE | sponge $OUTPUT_FILE
	fi

	# Count result is failed,skipped success in the json file
	success=$(jq '[.apps[] | select(.result == "success")] | length' $OUTPUT_FILE 2>/dev/null)
	fails=$(jq '[.apps[] | select(.result == "failed")] | length' $OUTPUT_FILE 2>/dev/null)
	skips=$(jq '[.apps[] | select(.result == "skipped")] | length' $OUTPUT_FILE 2>/dev/null)
	total=$(jq '.apps | length' $OUTPUT_FILE 2>/dev/null)
}

function gen_build_result_json() {
	if [ -f $ERR_FILE ]; then
		rm -f $ERR_FILE
	fi

	if [ ${skips} -ne 0 ]; then
		jq 'del(.apps[] | select(.result == "success" or .result == "failed"))' $OUTPUT_FILE >$SKIP_FILE
	fi

	if [ ${success} -ne 0 ]; then
		jq 'del(.apps[] | select(.result == "failed" or .result == "skipped"))' $OUTPUT_FILE >$SUCCESS_FILE
	fi

	if [ ${fails} -ne 0 ]; then
		jq 'del(.apps[] | select(.result == "success" or .result == "skipped"))' $OUTPUT_FILE >$ERR_FILE
	fi
}

function gen_excel() {
	# Check if the python3 exists
	hash python3 2>/dev/null || {
		echo -e "${RED}Waining: Command python3 not found${NC}"
		exit
	}

	if [ -f ${CWD}/json2excel.py ]; then
		python3 ${CWD}/json2excel.py $OUTPUT_FILE
	else
		echo "Fail to output excel file"
	fi
}

function print_results() {
	if [ ! -f $OUTPUT_FILE ]; then
		echo -e "${RED}Failed: File $OUTPUT_FILE not found${NC}"
		exit 1
	fi

	jq -r '.apps[] | [.name, .path, .cmd, .result] | @csv' "$OUTPUT_FILE" | sed 's/"//g' >"$CSV_FILE"
	sort -t, -k4,4r "$CSV_FILE" | awk 'NR==1; NR>1{print $0}' | column -t -s,
	echo -e "Total: $total, failed: $fails, skipped: $skips, success: $success"

	# post build result to gerrit server
	if [ -n "$GERRIT_CHANGE_NUMBER" ] && [ -n "$GERRIT_PATCHSET_NUMBER" ]; then
		message="Compile result: Total $total; Failed $fails; Skipped $skips; Success $success."
		gerrit_review "$GERRIT_CHANGE_NUMBER" "$GERRIT_PATCHSET_NUMBER" "$message"

		if [ ${fails} -ne 0 ]; then
			fail_apps=$(jq -r '.apps[] | select(.result == "failed") | .name' $ERR_FILE | tr '\n' ',')
			results=${fail_apps%,}
			message="Failed apps: $results"
			gerrit_review "$GERRIT_CHANGE_NUMBER" "$GERRIT_PATCHSET_NUMBER" "$message"
			sleep 1

			message="You can execute the command \"$CMD -f $INPUT_FILE_NAME\" or \"$CMD -f $INPUT_FILE_NAME -t <app-name>\" in the local directory \"tools/CI\" to check the CI result. For more details, use \"$CMD -h\"."
			gerrit_review "$GERRIT_CHANGE_NUMBER" "$GERRIT_PATCHSET_NUMBER" "$message"
		fi
	fi

	# Check if there are any failed
	if [ ${fails} -eq 0 ] && [ ${skips} -eq 0 ]; then
		echo -e "${GREEN}All builds succeeded${NC}"
	fi
}

function exit_build() {
	if [ ${fails} -eq 0 ]; then
		exit 0
	else
		exit 1
	fi
}

function main() {
	arg_check
	tools_check
	del_artifacts

	if [ -n "$TARGET_APP" ]; then
		build_target_app
	else
		build_all_apps
	fi

	get_results
	gen_build_result_json

	if [ $gen_result == true ]; then
		gen_excel
		print_results
	fi
	exit_build
}

main
exit 0

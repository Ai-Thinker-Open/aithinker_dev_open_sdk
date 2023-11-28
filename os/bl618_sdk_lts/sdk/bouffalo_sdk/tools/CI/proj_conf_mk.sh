#!/bin/bash

# Check the number of arguments
if [[ $# -ne 1 ]]; then
    echo "Usage: $0 input_file"
    exit 1
fi

# Parse the XML file
xml_file=$1
default_branch=""
count=0
output_file=$(basename "$xml_file" .xml)-conf.proj
while read -r line; do
    # Skip the commented lines and the lines with empty name
    if [[ $line == *"<!--"* ]] || [[ $line == *"name=\"\""* ]]; then
        continue
    fi

    if [[ $line == *"default"* ]]; then
        default_branch=$(echo "$line" | grep -o 'revision=".*"' | cut -d '"' -f 2)
    elif [[ $line == *"project"* ]]; then
        project_name=$(echo "$line" | grep -o 'name=".*"' | cut -d '"' -f 2)
        if [[ -z $project_name ]]; then
            continue
        fi

        if [[ $line == *"revision"* ]]; then
            project_branch=$(echo "$line" | grep -o 'revision=".*"' | cut -d '"' -f 2)
        else
            project_branch=$default_branch
        fi

        echo "p=$project_name" >> "$output_file"
        echo "b=$project_branch" >> "$output_file"

        count=$((count+1))
    fi
done < "$xml_file"

echo "Success,total $count projects in $output_file"

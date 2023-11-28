import json
import os
import sys
import pandas as pd

json_file = sys.argv[1]

# Read json file
with open(json_file, 'r') as f:
    data = json.load(f)

# Sace data to DataFrame
df = pd.DataFrame(data['apps'])

# Transform DataFrame to excel
output_file = os.path.splitext(json_file)[0] + '.xlsx'
df.to_excel(output_file, index=False)

print(f'Successfully output data to {output_file}.')


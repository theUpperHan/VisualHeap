import os, json, subprocess

def read_file(file_path):
    with open(file_path, 'r') as file:
        input_str = file.read()

    
    return input_str

def blocks_to_json(input_str):
    blocks = [block.strip("[] \n") for block in input_str.strip().split(']\n[')]
    parsed_blocks = []

    for block in blocks:
        block_dict = {}
        lines = block.split('\n')
        for line in lines:
            key, value = line.split(': ', 1)  # Split only on the first occurrence
            if value.isdigit():
                value = int(value)
            elif value.lower() == "true":
                value = True
            elif value.lower() == "false":
                value = False
            block_dict[key.strip()] = value
        parsed_blocks.append(block_dict)

    return json.dumps(parsed_blocks, indent=4)



# Convert the original string to JSON format

# result = subprocess.run(["pwd"], capture_output=True, text=True)
# print(result.stdout.strip())
subprocess.run(["make", "clean"], cwd=os.getcwd())
subprocess.run(["make"], cwd=os.getcwd())
raw_res = subprocess.run(["./printHeap"], capture_output=True, text=True, cwd=os.getcwd())
# print(raw_res.stdout.strip())
original_string = raw_res.stdout.strip()

json_output = blocks_to_json(original_string)

json_file_path = '../results/output.json'
with open(json_file_path, 'w') as json_file:
    json_file.write(json_output)
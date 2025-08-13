# Furina's Requiem

A simple program to help with multiplication problems.

## Compilation

### Windows
Run the batch file:
```batch
furina_compile.bat
```

### Linux/Mac
Run the shell script:
```bash
./furina_compile.sh
```

## Running the Program

1. The compiled program will be in the `bin` folder as "Se mettre sur son trente-et-un.exe"
2. Run the program and follow the on-screen instructions
3. Enter your answers when prompted

## Checking Your Answers

There are Python helper scripts in the `src` folder to:
- Generate random digits (`create_random_digit.py`)
- Verify multiplication results (`check_multiply.py`)

To check your answers:
```bash
python src/check_multiply.py
```

## Note
Make sure you have Python installed to use the checking scripts.
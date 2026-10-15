import csv
import sys
def input_int(prompt, default=None):
    while True:
        try:
            s = input(prompt).strip()
            if s == "" and default is not None:
                return default
            return int(s)
        except ValueError:
            print("Please enter an integer.")

def prompt_columns():
    cols = []
    n = input_int("Number of columns: ")
    for i in range(1, n+1):
        name = input(f"Name for column {i}: ").strip()
        if not name:
            name = f"col{i}"
        while True:
            t = input(f"Type for '{name}' (double / integer / string): ").strip().lower()
            if t in ("double", "d", "float"):
                t = "double"
                break
            if t in ("integer", "int", "i"):
                t = "integer"
                break
            if t in ("string", "s", "str"):
                t = "string"
                break
            print("Type must be one of: double, integer, string.")
        cols.append((name, t))
    return cols

def generate_row(cols, row_index):
    row = []
    for name, t in cols:
        if t == "integer":
            # simple increasing float sequence
            if row_index >= 10:
                row.append(int(f"1{row_index}"))
            else:
                row.append(int(f"10{row_index}"))
        else:  # integer or string
            index = input(f"enter {name}:")
            if t == "double":
                # cycle through integers
                row.append(float(index))
            else:  # string
                row.append(f"{index}")
        print(row)
    return row

def main():
    print("CSV Generator")
    cols = prompt_columns()
    if not cols:
        print("No columns provided. Exiting.")
        sys.exit(1)

    rows = input_int("Number of rows: ")
    outfile = input("Output filename (default 'output.csv'): ").strip() + ".csv"
    if not outfile:
        outfile = "output.csv"

    # characters to cycle for 'character' type
    try:
        with open(outfile, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            # header
            writer.writerow([name for name, _ in cols])
            # data rows (1-based index in sequence)
            for i in range(1, rows + 1):
                writer.writerow(generate_row(cols, i))
    except IOError as e:
        print(f"Failed to write file '{outfile}': {e}")
        sys.exit(1)

    print(f"Wrote {rows} rows to '{outfile}' with columns: {', '.join(name for name, _ in cols)}")

if __name__ == "__main__":
    main()

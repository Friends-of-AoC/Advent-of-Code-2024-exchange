use std::path::Path;
use std::{env, fs};

const SCHEMATICS_COLUMNS: usize = 5;
const SCHEMATICS_ROWS: usize = 7; // with header and trailer
const SCHEMATICS_ROWS_ACTUAL: usize = 5;
const SCHEMATICS_FIRST_ROW: usize = 0;
const SCHEMATICS_LAST_ROW: usize = SCHEMATICS_ROWS - 1;
const SCHEMATICS_SEPARATOR: &str = "\n\n";
const SCHEMATICS_LOCK_FIRST_ROW: &str = "#####";
const SCHEMATICS_LOCK_LAST_ROW: &str = ".....";
//const SCHEMATICS_KEY_FIRST_ROW: &str = ".....";
//const SCHEMATICS_KEY_LAST_ROW: &str = "#####";

const NULL: char = '?';
const FILLED: char = '#';
//const EMPTY: char = '.';

fn main() -> Result<(), String> {
    // Arg parsing
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        return Err(String::from("Usage: cargo run --bin day25 INPUT"));
    }

    // Load file into string
    let input_path = Path::new(&args[1]);
    let input = fs::read_to_string(input_path).unwrap();

    // Parse input
    let mut locks: Vec<[usize;SCHEMATICS_COLUMNS]> = Vec::new();
    let mut keys: Vec<[usize;SCHEMATICS_COLUMNS]> = Vec::new();
    let schematics = input.split(SCHEMATICS_SEPARATOR);
    for schema in schematics {
        let rows: Vec<&str> = schema.lines().collect();
        let mut columns: [[char;SCHEMATICS_ROWS_ACTUAL];SCHEMATICS_COLUMNS] = [[NULL;SCHEMATICS_ROWS_ACTUAL];SCHEMATICS_COLUMNS];
        for row in 1..SCHEMATICS_ROWS-1 { // skip first and last line
            let row_value = rows[row];
            for (column, column_value) in row_value.chars().enumerate() {
                columns[column][row-1] = column_value;
            }
        }
        let mut heights: [usize;SCHEMATICS_COLUMNS] = [0;SCHEMATICS_COLUMNS];
        for (column, column_value) in columns.iter().enumerate() {
            heights[column] = column_value.iter().filter(|&&x| x == FILLED).count();
        }
        // lock
        if rows[SCHEMATICS_FIRST_ROW] == SCHEMATICS_LOCK_FIRST_ROW && rows[SCHEMATICS_LAST_ROW] == SCHEMATICS_LOCK_LAST_ROW {
            // tbd
            locks.push(heights);
        // key
        } else {
            keys.push(heights);
        }
    }

    // Task 1
    // Analyze your lock and key schematics.
    // How many unique lock/key pairs fit together without overlapping in any column?
    let mut task1 = 0;
    for lock in &locks {
        for key in &keys {
            let mut overlap = false;
            for column in 0..SCHEMATICS_COLUMNS {
                if lock[column] + key[column] > SCHEMATICS_ROWS_ACTUAL {
                    overlap = true;
                    break;
                }
            }
            if !overlap {
                task1 += 1;
            }
        }
    }
    println!("Task 1: {}", task1);

    Ok(())
}

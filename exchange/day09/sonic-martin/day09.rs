use std::path::Path;
use std::{env, fs};

const EMPTY: u64 = u64::MAX;

fn main() -> Result<(), String> {
    // Arg parsing
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        return Err(String::from("Usage: cargo run --bin day08 INPUT"));
    }

    // Load input file into string
    let input_path = Path::new(&args[1]);
    let input = fs::read_to_string(input_path).unwrap();

    // Parse input
    let mut disk1: Vec<u64> = Vec::new();
    let mut id: u64 = 0;
    let mut is_file = true;
    let mut free_space: Vec<(usize, u32)> = Vec::new(); // for Task 2
    let mut files: Vec<(usize, u32, u64)> = Vec::new(); // for Task 2
    let mut index: usize = 0; // for Task 1
    for char in input.chars() {
        let length = char.to_digit(10).unwrap();

        let value: u64;
        if is_file {
            files.push((index, length, id));
            value = id;
            id += 1;
        } else {
            free_space.push((index, length));
            value = EMPTY;
        }

        for _ in 0..length {
            disk1.push(value);
        }

        is_file = !is_file; // alternate between files and free space
        index += length as usize;
    }
    let mut disk2 = disk1.clone(); // for Task 2

    // Task 1: Compact the amphipod's hard drive using the process he requested.
    // What is the resulting filesystem checksum?
    let mut left = 0;
    let mut right = disk1.len() - 1;
    while left < right {
        // Move left cursor to the next free space
        while disk1[left] != EMPTY {
            left += 1;
        }
        // Move right cursor to the next file block
        while disk1[right] == EMPTY {
            right -= 1;
        }
        // Swap to compact the disk
        disk1.swap(left, right);
        // Move cursors
        left += 1;
        right -= 1;
    }
    let mut checksum: u64 = 0;
    let mut i: usize = 0;
    while disk1[i] != EMPTY {
        let position: u64 = i as u64;
        let id: u64 = disk1[i];
        checksum += position * id;
        i += 1;
    }
    println!("Task 1: {}", checksum);

    // Task 2: Move whole files.
    while files.len() > 0 {
        let (file_index, file_length, file_id) = files.pop().unwrap();
        for i in 0..free_space.len() {
            let (free_index, free_length) = free_space[i];
            if free_length >= file_length {
                if free_index > file_index {
                    break;
                }
                for j in free_index..(free_index + file_length as usize) {
                    disk2[j] = file_id;
                }
                for j in file_index..(file_index + file_length as usize) {
                    disk2[j] = EMPTY;
                }
                let new_free_index = free_index + file_length as usize;
                let new_free_length = free_length - file_length;
                if new_free_length > 0 {
                    free_space[i] = (new_free_index, new_free_length);
                } else {
                    free_space.remove(i);
                }
                break;
            }
        }
    }
    let mut checksum2: u64 = 0;
    for j in 0..disk2.len() {
        if disk2[j] == EMPTY {
            continue;
        }
        let position: u64 = j as u64;
        let id: u64 = disk2[j];
        checksum2 += position * id;
    }
    println!("Task 2: {}", checksum2);

    Ok(())
}

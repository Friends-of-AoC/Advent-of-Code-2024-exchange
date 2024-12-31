// I first tried an approach, where I walked the one racetrack and while walking
// trying to branch off cheat paths. For the given test it was fine. For the real input
// I ended in a cycle/infinite loop.
//
// I peeked into Reddit and found out, that:
//   1. Walking the way once, and keep track of the path
//   2. Have a 2-layer nested loop of every point in the path with every other point
//   3. If the (manhattan) distance between both fits (e.g. 2)
//   4. And if the save fits (e.g. >=100)
//   5. Count + 1
// 
// See: https://www.reddit.com/r/adventofcode/comments/1hicdtb/comment/m2yl9oj

use std::collections::{HashSet};
use std::fs::File;
use std::io::BufRead;
use std::path::Path;
use std::{env, io};

const START: char = 'S';
const END: char = 'E';
const TRACK: char = '.';
const WALL: char = '#';

type Point = (usize, usize);

fn main() -> Result<(), String> {
    // Arg parsing
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        return Err(String::from("Usage: cargo run --bin day20 INPUT [TEST_FLAG]"));
    }

    // Real task (at least 100 saves) or test (all saves)
    let mut min_save = 100;
    if args.len() >= 3 {
        if &args[2] == "test" {
            min_save = 1;
        }
    }

    // Prepare file for reading
    let input_path = Path::new(&args[1]);
    let file =
        File::open(input_path).map_err(|_| String::from("Failed to open the INPUT file."))?;
    let reader = io::BufReader::new(file);
    let lines = reader.lines();

    // Load and parse input from file
    let mut map: Vec<Vec<char>> = Vec::new();
    let mut start: (usize, usize) = (0, 0);
    let mut end: (usize, usize) = (0, 0);
    for (y, line) in lines.enumerate() {
        map.push(Vec::new());
        for (x, c) in line.unwrap().chars().enumerate() {
            map[y].push(c);
            match map[y][x] {
                START => start = (x, y),
                END => end = (x, y),
                _ => {}
            }
        }
    }
    map[start.1][start.0] = TRACK;
    map[end.1][end.0] = TRACK;

    // Task 1
    // How many cheats would save you at least 100 picoseconds? (with cheat length = 2 picoseconds
    let path = walk(&map, start, end);
    let task1 = count_cheats(&path, 2, 2, min_save);
    println!("Task 1: {}", task1);

    // Task 2
    // How many cheats would save you at least 100 picoseconds? (with cheat length = 20 picoseconds
    let path = walk(&map, start, end);
    let task1 = count_cheats(&path, 2, 20, min_save);
    println!("Task 1: {}", task1);

    Ok(())
}

// Counts the possible cheats based on the given path. See comment at the top of this file.
// With this approach, the nested loops over each 2 possible points, we already handled that
// we don't count the same cheat (identified by start/end position of the cheat) more than once 
// (important for task 2).
fn count_cheats(path: &Vec<Point>, min_manhattan_distance: usize, max_manhattan_distance: usize, min_save: i32) -> u32 {
    let mut cheats = 0;
    for (i, point1) in path.iter().enumerate() {
        for (j, point2) in path.iter().enumerate() {
            if i == j {
                continue;
            }

            let manhattan_distance = point1.0.abs_diff(point2.0) + point1.1.abs_diff(point2.1);
            let save = j as i32 - i as i32 - manhattan_distance as i32;

            if manhattan_distance >= min_manhattan_distance && manhattan_distance <= max_manhattan_distance && save >= min_save {
                cheats += 1;
            }
        }
    }
    cheats
}

// Depth-first search from the current position to the end position.
fn walk(
    map: &Vec<Vec<char>>,
    start_pos: (usize, usize),
    end_pos: (usize, usize),
) -> Vec<Point> {
    let mut visited = HashSet::new();
    let mut path = Vec::new();

    let mut stack = Vec::new();
    stack.push(start_pos);

    while let Some(current_pos) = stack.pop() {
        if current_pos == end_pos {
            path.push(current_pos);
            break
        }

        if visited.contains(&current_pos) {
            continue
        }

        visited.insert(current_pos);
        path.push(current_pos);

        for neighbour_pos in neighbours(map, current_pos) {
            stack.push(neighbour_pos);
        }
    }

    path
}

fn neighbours(map: &Vec<Vec<char>>, pos: (usize, usize)) -> Vec<(usize, usize)> {
    let mut neighbours = Vec::new();
    let directions: [(i64, i64); 4] = [(1, 0), (0, 1), (-1, 0), (0, -1)];
    for (dx, dy) in directions {
        let neighbour_pos= (pos.0 as i64 + dx, pos.1 as i64 + dy);

        // out of bounds
        if neighbour_pos.0 < 0
            || neighbour_pos.0 >= map[0].len() as i64
            || neighbour_pos.1 < 0
            || neighbour_pos.1 >= map.len() as i64
        {
            continue;
        }

        if map[neighbour_pos.1 as usize][neighbour_pos.0 as usize] == WALL {
            continue;
        }

        neighbours.push((neighbour_pos.0 as usize, neighbour_pos.1 as usize));
    }

    neighbours
}


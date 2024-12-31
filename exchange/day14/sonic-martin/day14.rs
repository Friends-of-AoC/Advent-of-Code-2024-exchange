use std::fs::File;
use std::io::BufRead;
use std::path::Path;
use std::{env, io};
use itertools::Itertools;

#[derive(Clone, Debug)]
struct Robot {
    p_x: i64,
    p_y: i64,
    v_x: i64,
    v_y: i64,
}

fn main() -> Result<(), String> {
    // Arg parsing
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        return Err(String::from(
            "Usage: cargo run --bin day14 INPUT [TEST_FLAG]",
        ));
    }

    // Real input (71x71 matrix) or test (7x7 matrix)
    let mut max_row = 103;
    let mut max_col = 101;
    if args.len() >= 3 {
        if &args[2] == "test" {
            max_row = 7;
            max_col = 11;
        }
    }

    // Prepare file for reading
    let input_path = Path::new(&args[1]);
    let file =
        File::open(input_path).map_err(|_| String::from("Failed to open the INPUT file."))?;
    let reader = io::BufReader::new(file);
    let lines = reader.lines().collect::<Result<Vec<_>, _>>().unwrap();

    // Parse input
    let mut robots = Vec::new();
    for line in &lines {
        let parts = line.split_once(' ').unwrap();
        let p = parts.0.split_once('=').unwrap().1.split_once(',').unwrap();
        let v = parts.1.split_once('=').unwrap().1.split_once(',').unwrap();
        let p_x = p.0.parse().unwrap();
        let p_y = p.1.parse().unwrap();
        let v_x = v.0.parse().unwrap();
        let v_y = v.1.parse().unwrap();
        robots.push(Robot { p_x, p_y, v_x, v_y });
    }

    // Task 1:
    // Predict the motion of the robots in your list within a space which is 101 tiles wide and
    // 103 tiles tall.
    // What will the safety factor be after exactly 100 seconds have elapsed?
    let mut robots1 = Vec::new();
    for robot in &robots {
        let rob = simulate_robot(&robot, 100, max_row, max_col);
        robots1.push(rob);
    }
    let task1 = safety_factor(&robots1, max_row, max_col);
    println!("Task 1: {}", task1);

    // Task 2:
    // What is the fewest number of seconds that must elapse for the robots to display the
    // Easter egg? (forming a Christmas tree)
    // 
    // Remark: Two ways to find the number:
    // (1) Visualized -> Print in a loop every output and check it. Tried that with numbers
    // up to 10.000, but oversaw it personally.
    // (2) Programmatically -> Reddit gave the hint, that the tree is there when on every coordinate
    // just one robot is there.
    for i in 0.. {
        let mut robots2 = Vec::new();
        for robot in &robots {
            let rob = simulate_robot(&robot, i, max_row, max_col);
            robots2.push(rob);
        }
        let all_unique = robots2.iter().map(|r| (r.p_x, r.p_y)).collect::<Vec<_>>().iter().all_unique();
        if all_unique {
            println!("Task 2: {}", i);
            break;
        }
    }

    Ok(())
}

fn simulate_robot(robot: &Robot, seconds: i64, max_row: i64, max_col: i64) -> Robot {
    let mut p_x = (robot.p_x + robot.v_x * seconds) % max_col;
    if p_x < 0 {
        p_x = (max_col + p_x) % max_col;
    }
    // p=2,4 v=2,-3 -> korekt after 2sec: p=6,5
    let mut p_y = (robot.p_y + robot.v_y * seconds) % max_row;
    if p_y < 0 {
        p_y = (max_row + p_y) % max_row;
    }
    Robot {
        p_x,
        p_y,
        v_x: robot.v_x,
        v_y: robot.v_y,
    }
}

fn safety_factor(robots: &Vec<Robot>, max_row: i64, max_col: i64) -> u64 {
    let mut factors: [u64;4] = [0, 0, 0, 0];
    let horizontal_center = max_col / 2;
    let vertical_center = max_row / 2;
    for robot in robots {
        let quadrant;
        if robot.p_x < horizontal_center && robot.p_y < vertical_center {
            quadrant = 0;
        } else if robot.p_x > horizontal_center && robot.p_y < vertical_center {
            quadrant = 1;
        } else if robot.p_x < horizontal_center && robot.p_y > vertical_center {
            quadrant = 2;
        } else if robot.p_x > horizontal_center && robot.p_y > vertical_center {
            quadrant = 3;
        } else {
            continue;
        }
        factors[quadrant] += 1;
    }
    factors[0] * factors[1] * factors[2] * factors[3]
}

fn print(robots: &Vec<Robot>, max_row: i64, max_col: i64) {
    let mut grid: Vec<Vec<u32>> = Vec::new();
    for row in 0..max_row as usize {
        grid.push(Vec::new());
        for _ in 0..max_col as usize {
            grid[row].push(0);
        }
    }
    for robot in robots {
        grid[robot.p_y as usize][robot.p_x as usize] += 1;
    }
    for row in 0..max_row as usize {
        for col in 0..max_col as usize {
            let val = grid[row][col];
            if val == 0 {
                print!(" ");
            } else {
                print!("{}", val);
            }
        }
        println!();
    }
}
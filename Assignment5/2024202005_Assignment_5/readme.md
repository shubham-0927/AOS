# AOS Assingment 5 Report

## Problem Statement

The goal of this program is to simulate the usage of washing machines in a student dormitory with the following specifications:

1. **N students** arrive throughout the day to use **M washing machines**.
2. Each student:
   - Has an **arrival time** \( T_i \) indicating how long after program execution they will arrive.
   - Needs a **washing time** \( W_i \) to complete washing.
   - Has a **patience time** \( P_i \), after which they will leave if no machine is available.
3. The simulation must:
   - Report each event (arrival, wash start, wash complete, and leaving without washing) with specific colors.
   - Determine whether more machines are needed, based on if 25% or more students leave without washing.
4. The program should be deadlock-free, avoid busy waiting, and use threads, semaphores, and mutexes effectively.

## Solution Overview

### Algorithm

1. **Student Struct and Initialization**:
   - Each student is represented by a `Student` struct that includes `arrival_time`, `wash_time`, `patience`, and an `id`.
2. **Reordering Students**:
   - Students are reordered by `arrival_time` and `id` to ensure deterministic processing.
3. **Multi-threaded Simulation**:
   - Each student is represented by a thread that:
     - Waits until its arrival time.
     - Tries to acquire a washing machine (waiting within their patience time).
     - Either starts washing if a machine is available or leaves if they run out of patience.

### Implementation Details

1. **Student Thread Execution**:
   - **Arrival Wait**: A busy-wait simulates the arrival time using `time(0)`.
   - **Washing**:
     - If a washing machine is available within patience time, the student begins washing.
     - Washing is simulated by another busy-wait using `time(0)`.
     - After washing, the machine is marked as available.
   - **Leaving**:
     - If a machine was not available within patience, the student increments the `left_unwashed_count` and leaves.

2. **Synchronization Mechanisms**:
   - **Mutex and Condition Variable**:
     - A mutex (`mtx`) protects shared resources, and a condition variable (`cv`) coordinates machine availability.
     - Another mutex (`left_unwashed_mutex`) protects the `left_unwashed_count` variable.
   - **Thread Array**:
     - A dynamic array of threads is created to manage and join all student threads.

3. **Output**:
   - Events are printed with specific colors: white for arrival, green for start of washing, yellow for completion, and red for leaving without washing.
   - After simulation, a percentage check determines if more machines are needed, printed as "Yes" or "No."

### Code Structure

The code consists of the following primary functions:

- `rearrange()`: Sorts students by arrival time and id.
- `simulate_student()`: Simulates each student's arrival, waiting, and washing process.
- `start_simulation()`: Sets up threads and starts the simulation.
- `main()`: Reads input, initializes students, and triggers the simulation.

Here’s an additional "Steps to Run" section for the README:

---

## Steps to Run

1. **Compile the Code**: Ensure you have a C++ compiler (e.g., `g++`).
   ```bash
   g++ -std=c++11 solution.cpp -lpthread
   ```
   - The `-pthread` flag is required to enable multi-threading support.

2. **Run the Program**: Execute the compiled program with the following command.
   ```bash
   ./a.out
   ```

3. **Provide Input**:
   - Enter the number of students (`N`) and the number of washing machines (`M`) as two integers.
   - For each student, provide three integers: 
     - `T_i` (arrival time in seconds after execution),
     - `W_i` (washing time in seconds), and 
     - `P_i` (patience time in seconds).

   **Example Input:**
   ```
   5 2
   2 3 5
   4 6 3
   1 4 2
   5 2 4
   3 5 6
   ```

4. **View Output**:
   - The program will display color-coded messages for each event:
     - White for arrival
     - Green for starting washing
     - Yellow for completion
     - Red for leaving without washing
   - It will also output:
     - The total number of students who left without washing.
     - A final line with "Yes" if more machines are needed (if ≥25% of students left unwashed) or "No" otherwise.

---

This section provides users with a clear, step-by-step guide to compile, run, and interpret the output of the simulation.

## Conclusion

After the simulation, the program checks if the number of students who left without washing exceeds 25% of the total. If so, it outputs "Yes," indicating more machines are needed; otherwise, it outputs "No."

This program meets the problem's requirements, using multi-threading and synchronization to handle shared resources and providing clear, color-coded output for event tracking. The code avoids deadlocks through structured locking and uses condition variables to eliminate busy waiting when waiting for machine availability.
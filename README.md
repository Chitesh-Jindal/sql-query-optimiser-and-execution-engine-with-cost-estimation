# Relational Algebra Based Query Optimizer and Execution Engine

A C++ DBMS project that parses simplified SQL queries, converts them into Relational Algebra trees, applies query optimization using selection pushdown, and executes the optimized query on CSV-based tables.

---

## Project Overview

This project demonstrates the internal working of a basic database query processor.

Instead of directly executing SQL, the system first converts a SQL query into a Relational Algebra tree. The tree is then optimized using query optimization rules and finally executed on CSV files that act as database tables.

The project is built as a learning-focused DBMS engine to understand how SQL parsing, relational algebra, query optimization, and query execution work together.

---

## Main Objectives

- Parse simplified SQL queries.
- Generate and display the original Relational Algebra tree.
- Apply query optimization using selection pushdown.
- Display the optimized Relational Algebra tree.
- Execute the optimized query plan.
- Display the final query result.

---

## Features

- SQL query parsing
- Relational Algebra tree generation
- Original RA tree display
- Optimized RA tree display
- Selection pushdown optimization
- Execution of relational algebra operators
- CSV-based table loading
- Support for multiple `AND` conditions in `WHERE`
- Basic numeric and string condition handling
- Final query result display in tabular format

---

## Technologies Used

- C++
- STL vectors
- STL maps
- File handling
- CSV files
- Relational Algebra
- Query Optimization

---

## Project Structure

```text
ra-query-optimizer-execution-engine/
│
├── main.cpp
├── EMP.csv
├── DEPT.csv
├── PROJECT.csv
├── CLIENT.csv
├── ASSIGNMENT.csv
├── WORKS_ON.csv
├── sample_queries.txt
├── README.md
├── LICENSE
└── .gitignore
```

---

## Supported SQL Format

The engine supports simplified SQL queries in the following formats:

```sql
SELECT column1,column2 FROM TABLE WHERE condition;
```

```sql
SELECT column1,column2 FROM TABLE1 JOIN TABLE2 ON TABLE1.column=TABLE2.column WHERE condition;
```

Example:

```sql
SELECT EMP.NAME,DEPT.NAME FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID WHERE EMP.AGE>25;
```

---

## Important Input Rules

- SQL keywords should be written in uppercase.
- Column names should match the CSV headers exactly.
- Table-qualified column names should be used.

Example:

```text
EMP.NAME
DEPT.ID
```

- String values should use straight single quotes.

Correct:

```sql
EMP.CITY='DELHI'
```

Incorrect:

```sql
EMP.CITY=‘DELHI’
```

- Queries should end with a semicolon.

---

## How to Run

### 1. Compile the program

```bash
g++ main.cpp -o engine
```

### 2. Run the executable

```bash
./engine
```

### 3. Enter a SQL query

```sql
SELECT EMP.NAME,EMP.AGE,EMP.SALARY FROM EMP WHERE EMP.AGE>25 AND EMP.SALARY>60000;
```

---

## Sample Queries

### Query 1: Simple selection with multiple conditions

```sql
SELECT EMP.NAME,EMP.AGE,EMP.SALARY FROM EMP WHERE EMP.AGE>25 AND EMP.SALARY>60000;
```

### Query 2: String condition

```sql
SELECT EMP.NAME,EMP.CITY FROM EMP WHERE EMP.CITY='DELHI';
```

### Query 3: Join with selection condition

```sql
SELECT EMP.NAME,DEPT.NAME FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID WHERE EMP.AGE>25;
```

### Query 4: Join with department filter

```sql
SELECT EMP.NAME,DEPT.NAME,DEPT.LOCATION FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID WHERE DEPT.LOCATION='MUMBAI';
```

---

## Example Output

For the query:

```sql
SELECT EMP.NAME,DEPT.NAME FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID WHERE EMP.AGE>25;
```

The engine first displays the original Relational Algebra tree:

```text
Original RA tree :
PROJECT(EMP.NAME,DEPT.NAME)
  SELECT(EMP.AGE>25)
    JOIN(EMP.DEPTID=DEPT.ID)
      TABLE(EMP)
      TABLE(DEPT)
```

After optimization, it displays the optimized tree:

```text
Optimised RA tree :
PROJECT(EMP.NAME,DEPT.NAME)
  JOIN(EMP.DEPTID=DEPT.ID)
    SELECT(EMP.AGE>25)
      TABLE(EMP)
    TABLE(DEPT)
```

Then it executes the optimized tree and displays the final result.

---

## Optimization Technique Implemented

### Selection Pushdown

Selection pushdown is a query optimization technique where filtering conditions are moved closer to the base tables before performing join operations.

This reduces the number of rows participating in the join operation and improves query execution efficiency.

Before optimization:

```text
PROJECT
  SELECT
    JOIN
      TABLE
      TABLE
```

After optimization:

```text
PROJECT
  JOIN
    SELECT
      TABLE
    TABLE
```

This optimization is applied when the `WHERE` condition belongs to only one table.

---

## Execution Engine

The execution engine recursively evaluates the Relational Algebra tree.

Each node type performs a specific operation:

| Node Type | Function |
|---|---|
| TABLE | Loads a relation from the CSV-based database |
| SELECT | Filters rows based on conditions |
| PROJECT | Selects only required columns |
| JOIN | Combines two relations based on a join condition |

---

## CSV-Based Database

The project uses CSV files as database tables.

Each CSV file contains:

- First row: column names
- Remaining rows: table records

Example:

```text
EMP.ID,EMP.NAME,EMP.DEPTID,EMP.AGE,EMP.SALARY,EMP.CITY
1,Aman,10,24,50000,DELHI
```

---

## Current Limitations

- Only simplified SQL syntax is supported.
- SQL keywords should be written in uppercase.
- Nested queries are not supported.
- Aggregate functions are not supported.
- `ORDER BY` and `GROUP BY` are not supported.
- Projection pushdown is not fully implemented.
- The parser is designed for controlled academic demo queries.

---

## Future Scope

- Projection pushdown optimization
- Support for lowercase SQL keywords
- Support for aliases
- Support for aggregate functions
- Support for `ORDER BY` and `GROUP BY`
- Improved SQL parser
- Better error handling
- GUI or web-based interface for query input and output

---

## Learning Outcome

This project helped in understanding:

- How SQL queries are internally represented
- How Relational Algebra trees are generated
- How query optimization improves execution
- How selection pushdown works
- How relational operators can be executed using C++
- How a basic DBMS query engine can be designed from scratch

---

## Author

**Chitesh Jindal**

---

## License

This project is licensed under the MIT License.

# Cost-Based Relational Algebra Query Optimizer and Execution Engine

A C++ DBMS project that parses simplified SQL queries, converts them into Relational Algebra trees, applies query optimization using selection pushdown, estimates query execution cost, and executes the optimized query on CSV-based tables.

---

## Project Overview

This project demonstrates the internal working of a basic database query processor.

Instead of directly executing SQL, the system first converts a simplified SQL query into a Relational Algebra tree. The original tree is analyzed, its estimated execution cost is calculated, and then query optimization is applied. After optimization, the new tree and its cost are displayed, allowing comparison between the original and optimized query plans.

CSV files are used as database tables, making the project easy to run and understand without requiring an external DBMS.

---

## Main Objectives

* Parse simplified SQL queries.
* Generate the original Relational Algebra tree.
* Estimate the cost of the original query tree.
* Apply query optimization using selection pushdown.
* Generate the optimized Relational Algebra tree.
* Estimate the cost of the optimized query tree.
* Compare original cost vs optimized cost.
* Execute the optimized query plan on CSV-based tables.
* Display the final query result.

---

## Features

* SQL query parsing
* Relational Algebra tree generation
* Original and optimized query tree display
* Selection pushdown optimization
* Cost estimation for original query plan
* Cost estimation for optimized query plan
* Page-based I/O cost model
* Comparison of original cost vs optimized cost
* Execution of relational algebra operators
* CSV-based table loading
* Support for multiple `AND` conditions in `WHERE`
* Basic numeric and string condition handling
* Final query result display in tabular format

---

## Technologies Used

* C++
* STL vectors
* STL maps
* File handling
* CSV files
* Relational Algebra
* Query Optimization
* Cost-Based Query Evaluation

---

## Project Structure

```text
Cost-Based-Relational-Algebra-Optimizer/
│
├── main.cpp
├── EMP.csv
├── DEPT.csv
├── PROJECT.csv
├── CLIENT.csv
├── ASSIGNMENT.csv
├── WORKS_ON.csv
├── sample_queries.txt
├── COST_CALCULATION.md
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

* SQL keywords should be written in uppercase.
* Column names should match the CSV headers exactly.
* Table-qualified column names should be used.

Example:

```text
EMP.NAME
DEPT.ID
```

* String values should use straight single quotes.

Correct:

```sql
EMP.CITY='DELHI'
```

Incorrect:

```sql
EMP.CITY=‘DELHI’
```

* Queries should end with a semicolon.

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

### Query 1: Simple Selection

```sql
SELECT EMP.NAME,EMP.AGE FROM EMP WHERE EMP.AGE>25;
```

### Query 2: Selection with Multiple Conditions

```sql
SELECT EMP.NAME,EMP.AGE,EMP.SALARY FROM EMP WHERE EMP.AGE>25 AND EMP.SALARY>60000;
```

### Query 3: String Condition

```sql
SELECT EMP.NAME,EMP.CITY FROM EMP WHERE EMP.CITY='DELHI';
```

### Query 4: Join with Selection

```sql
SELECT EMP.NAME,DEPT.NAME FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID WHERE EMP.AGE>25;
```

### Query 5: Join with Conditions on Both Tables

```sql
SELECT EMP.NAME,EMP.SALARY,DEPT.NAME,DEPT.LOCATION FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID WHERE EMP.AGE>25 AND EMP.SALARY>60000 AND DEPT.LOCATION='BANGALORE';
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

Then it calculates the estimated cost of the original query tree:

```text
Original Cost: 97 pages
```

After applying selection pushdown, the optimized tree is displayed:

```text
Optimized RA tree :
PROJECT(EMP.NAME,DEPT.NAME)
  JOIN(EMP.DEPTID=DEPT.ID)
    SELECT(EMP.AGE>25)
      TABLE(EMP)
    TABLE(DEPT)
```

Then it calculates the estimated cost of the optimized query tree:

```text
Optimized Cost: 83 pages
```

Finally, the optimized query tree is executed and the final result is displayed.

---

## Cost Estimation Model

The project uses a simplified page-based I/O cost model.

Cost represents the estimated number of disk pages read or written during query execution. It is not exact runtime, but an approximation used to compare different query plans.

The basic cost formula is:

```text
pages = ceil((number_of_rows × number_of_columns × column_size) / page_size)
```

The cost model considers:

* Number of rows produced by each operator
* Number of output columns
* Estimated number of pages read or written
* Effect of filtering rows before join operations

This allows the system to compare the original query plan with the optimized query plan.

---

## Optimization Technique Implemented

### Selection Pushdown

Selection pushdown is a query optimization technique where filtering conditions are moved closer to the base tables before performing join operations.

This reduces the number of rows participating in the join operation and can reduce the estimated execution cost.

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

This optimization is applied when a `WHERE` condition belongs to only one table.

---

## Execution Engine

The execution engine recursively evaluates the Relational Algebra tree.

Each node type performs a specific operation:

| Node Type | Function                                         |
| --------- | ------------------------------------------------ |
| TABLE     | Loads a relation from the CSV-based database     |
| SELECT    | Filters rows based on conditions                 |
| PROJECT   | Selects only required columns                    |
| JOIN      | Combines two relations based on a join condition |

---

## CSV-Based Database

The project uses CSV files as database tables.

Each CSV file contains:

* First row: column names
* Remaining rows: table records

Example:

```text
EMP.ID,EMP.NAME,EMP.DEPTID,EMP.AGE,EMP.SALARY,EMP.CITY
1,Aman,10,24,50000,DELHI
```

---

## Current Limitations

* Only simplified SQL syntax is supported.
* SQL keywords should be written in uppercase.
* Nested queries are not supported.
* Aggregate functions are not supported.
* `ORDER BY` and `GROUP BY` are not supported.
* Projection pushdown is not fully implemented.
* The parser is designed for controlled academic demo queries.
* The cost model is simplified and used for comparison, not exact runtime prediction.

---

## Future Scope

* Projection pushdown optimization
* Support for lowercase SQL keywords
* Support for table aliases
* Support for aggregate functions
* Support for `ORDER BY` and `GROUP BY`
* Improved SQL parser
* Improved error handling
* More realistic cost estimation model
* GUI or web-based interface for query input and output

---

## Learning Outcome

This project helped in understanding:

* How SQL queries are internally represented
* How Relational Algebra trees are generated
* How query optimization improves execution
* How selection pushdown works
* How query cost can be estimated using a page-based model
* How relational operators can be executed using C++
* How a basic DBMS query engine can be designed from scratch

---

## Author

**Chitesh Jindal**

---

## License

This project is licensed under the MIT License.

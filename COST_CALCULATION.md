# Cost Calculation Model

This project includes a page-based cost estimation module to compare the cost of the original Relational Algebra tree and the optimized Relational Algebra tree.

The cost does not represent exact runtime. Instead, it represents the estimated number of disk pages read or written during query execution. Since disk I/O is usually one of the most expensive operations in DBMS query processing, lower page cost indicates a more efficient query plan.

---

## Purpose of Cost Calculation

The main purpose of cost calculation in this project is to show how query optimization improves execution efficiency.

The engine calculates:

```text
Original Cost
```

for the original Relational Algebra tree, and:

```text
Optimized Cost
```

for the optimized tree after applying selection pushdown.

This allows direct comparison between the unoptimized and optimized query plans.

---

## Basic Assumptions

The cost model uses the following assumptions:

```cpp
const int pagesize = 256;
const int columnsize = 8;
```
This project assumes intermediate result materialization.

### Page Size

A page is the basic unit of disk I/O.

In real database systems, page size is often larger, such as 4096 bytes. However, this project uses a smaller page size of 256 bytes so that cost differences are clearly visible on small CSV datasets.

### Column Size

Each column is assumed to occupy 8 bytes.

This is a simplified assumption used to estimate tuple size without handling different data types separately.

---

## Tuple Size

The size of one tuple is estimated as:

```text
tupleSize = numberOfColumns × columnSize
```

For example, if a relation has 5 columns:

```text
tupleSize = 5 × 8 = 40 bytes
```

---

## Page Calculation Formula

The number of pages required to store a relation is calculated as:

```text
pages = ceil((rows × columns × columnSize) / pageSize)
```

In code, this can be written without floating point arithmetic as:

```cpp
long long costFormula(long long rows, long long columns){
    if(rows <= 0 || columns <= 0)
        return 0;

    return (rows * columns * columnsize + pagesize - 1) / pagesize;
}
```

This returns the number of pages required for a relation produced by any operator.

---

## Cost Information Stored per Node

Each Relational Algebra node returns a `costinfo` object.

```cpp
class costinfo{
public:
    long long estimatedRows = 0;
    vector<string> outputColumns;
    long long outputPages = 0;
    long long totalCost = 0;
};
```

### Meaning of Each Field

| Field           | Meaning                                                        |
| --------------- | -------------------------------------------------------------- |
| `estimatedRows` | Number of rows produced by the current node                    |
| `outputColumns` | Columns produced by the current node                           |
| `outputPages`   | Number of pages produced by the current node                   |
| `totalCost`     | Total cost required to execute the subtree rooted at this node |

---

# Operator Cost Models

---

## 1. TABLE Cost

A `TABLE` node represents scanning a base table from the CSV-based database.

### Formula

```text
tableCost = tablePages
```

### Explanation

The table must be read from disk, so the cost is equal to the number of pages occupied by the table.

### Code Logic

```cpp
result.estimatedRows = r.row.size();
result.outputColumns = r.column;
result.outputPages = costFormula(result.estimatedRows, result.outputColumns.size());
result.totalCost = result.outputPages;
```

---

## 2. SELECT Cost

A `SELECT` node filters rows based on a condition.

### Formula

```text
selectCost = childCost + childOutputPages + selectedOutputPages
```

### Explanation

Selection needs to:

1. Execute the child subtree.
2. Read the child output.
3. Write the filtered output.

So the total cost includes the previous child cost, the cost of reading the child result, and the cost of writing the selected result.

### Code Logic

```cpp
result.estimatedRows = selected.row.size();
result.outputColumns = childcost.outputColumns;
result.outputPages = costFormula(result.estimatedRows, result.outputColumns.size());

result.totalCost =
    childcost.totalCost +
    childcost.outputPages +
    result.outputPages;
```

---

## 3. PROJECT Cost

A `PROJECT` node keeps only the required columns.

### Formula

```text
projectCost = childCost + childOutputPages + projectedOutputPages
```

### Explanation

Projection needs to:

1. Execute the child subtree.
2. Read the child output.
3. Write the projected output with fewer columns.

Projection usually reduces the number of columns, so the projected output may require fewer pages.

### Code Logic

```cpp
result.estimatedRows = childcost.estimatedRows;
vector<string> requiredColumns = splitcomma(root->value);

for(string col : requiredColumns){
    if(col exists in childcost.outputColumns)
        result.outputColumns.push_back(col);
    else
        report invalid column error;
}
result.outputPages = costFormula(result.estimatedRows, result.outputColumns.size());

result.totalCost =
    childcost.totalCost +
    childcost.outputPages +
    result.outputPages;
```

---

## 4. JOIN Cost

A `JOIN` node combines two relations based on a join condition.

This project uses a simplified page nested loop join cost model.

### Formula

```text
joinCost = leftCost + rightCost + joinProcessingCost + joinOutputPages
```

Where:

```text
joinProcessingCost = min(
    leftPages + leftPages × rightPages,
    rightPages + rightPages × leftPages
)
```

### Explanation

In a page nested loop join, one relation is chosen as the outer relation and the other relation is scanned repeatedly as the inner relation.

If the left relation is outer:

```text
cost = leftPages + leftPages × rightPages
```

If the right relation is outer:

```text
cost = rightPages + rightPages × leftPages
```

The minimum of both is taken because the optimizer assumes the smaller input can be chosen as the outer relation.

### Code Logic

```cpp
long long leftOuterCost =
    leftcost.outputPages + leftcost.outputPages * rightcost.outputPages;

long long rightOuterCost =
    rightcost.outputPages + rightcost.outputPages * leftcost.outputPages;

long long joinProcessingCost = min(leftOuterCost, rightOuterCost);

result.totalCost =
    leftcost.totalCost +
    rightcost.totalCost +
    joinProcessingCost +
    result.outputPages;
```

---

# Why Selection Pushdown Reduces Cost

Selection pushdown moves filtering conditions closer to the base table before performing joins.

---

## Before Optimization

```text
PROJECT
  SELECT
    JOIN
      TABLE EMP
      TABLE DEPT
```

In this case, the join happens first. This means many rows participate in the join, and filtering happens after the join.

---

## After Optimization

```text
PROJECT
  JOIN
    SELECT
      TABLE EMP
    TABLE DEPT
```

In this case, filtering happens before the join. This reduces the number of rows and pages entering the join.

Since join cost depends on input pages, reducing input pages reduces total join cost.

---

## Example

Suppose before optimization:

```text
EMP pages = 10
DEPT pages = 4
```

Join processing cost:

```text
min(10 + 10 × 4, 4 + 4 × 10)
= min(50, 44)
= 44 pages
```

After selection pushdown:

```text
Filtered EMP pages = 3
DEPT pages = 4
```

Join processing cost:

```text
min(3 + 3 × 4, 4 + 4 × 3)
= min(15, 16)
= 15 pages
```

So selection pushdown reduces the join processing cost from:

```text
44 pages to 15 pages
```

---

# Final Cost Model Summary

| Operator | Cost Formula                                                                                                            |
| -------- | ----------------------------------------------------------------------------------------------------------------------- |
| TABLE    | `tablePages`                                                                                                            |
| SELECT   | `childCost + childOutputPages + selectedOutputPages`                                                                    |
| PROJECT  | `childCost + childOutputPages + projectedOutputPages`                                                                   |
| JOIN     | `leftCost + rightCost + min(leftPages + leftPages × rightPages, rightPages + rightPages × leftPages) + joinOutputPages` |

---

# Example Output

For a query such as:

```sql
SELECT EMP.NAME,DEPT.NAME,EMP.SALARY 
FROM EMP JOIN DEPT ON EMP.DEPTID=DEPT.ID 
WHERE EMP.AGE>25 AND EMP.SALARY>60000;
```

The engine displays:

```text
Original RA tree :
PROJECT(EMP.NAME,DEPT.NAME,EMP.SALARY)
  SELECT(EMP.AGE>25 AND EMP.SALARY>60000)
    JOIN(EMP.DEPTID=DEPT.ID)
      TABLE(EMP)
      TABLE(DEPT)

Original Cost: 89 pages
```

After optimization:

```text
Optimized RA tree :
PROJECT(EMP.NAME,DEPT.NAME,EMP.SALARY)
  JOIN(EMP.DEPTID=DEPT.ID)
    SELECT(EMP.AGE>25 AND EMP.SALARY>60000)
      TABLE(EMP)
    TABLE(DEPT)

Optimized Cost: 69 pages
```

This shows that the optimized query plan has lower estimated cost.

---

# Limitations of the Cost Model

This cost model is simplified for academic understanding.

Current limitations:

* Fixed column size is assumed.
* All data types are treated as equal size.
* Indexes are not considered.
* Buffer memory is not considered.
* Join cost is based on page nested loop join only.
* Actual runtime may differ from estimated cost.
* Selection row count is calculated using the CSV data instead of statistical selectivity estimation.

---

# Future Improvements

The cost model can be improved by adding:

* Different sizes for different data types
* Table statistics
* Selectivity estimation
* Index scan cost
* Block nested loop join cost
* Hash join cost
* Sort-merge join cost
* Buffer-aware cost estimation
* Projection pushdown cost benefits

---

# Learning Outcome

This cost calculation module helped in understanding how DBMS query optimizers compare different query plans.

The project demonstrates that optimization is not only about changing the query tree, but also about reducing the estimated execution cost of the query plan.

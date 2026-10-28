# Project 3 Design Log
**Author:** <Elizabeth Szatanek> · **P2 code used:** <my own>
**Tools used:** <ChatGPT was used to help me understand the projects requirements and break it down into smaller, more manageable parts. I also used it to help me debug and test. VS Code and WSL terminal were used to create and test this project.>

## 1. Hash table
I chose to use separate chaining for my ToolRegistry because it allows multiple tools to occupy the same bucket without requiring the registry to find another open position. Each bucket contains a vector of unique_ptr<Tool> objects. I used the FNV-1a hash function to distribute tool names across the buckets. The registry starts with eight buckets and doubles whenever the next insertion would cause the load factor to exceed 1.0. I chose this threshold to keep the number of tools per bucket relatively low while avoiding unnecessary resizing.

My 10,000-tool test resulted in 16,384 buckets, a load factor of 0.610352, and a maximum chain length of 6. I compared this with the alternative hash function, s[0] + s.size(), using the same bucket count. The alternative produced a maximum chain length of 9,000. Since all the generated names begin with tool_, the alternative hash does not distribute them well. The results show that FNV-1a distributes the tool names much more evenly than the alternative hash function.

The average insertion time is O(1) under normal hash-distribution assumptions. Most insertions only require checking a bucket and adding a tool. When resizing occurs, the registry must redistribute its existing tools, making that individual insertion more expensive. However, doubling the bucket count means resizing does not happen on every insertion, so the cost is spread across many insertions. Separate chaining also means multiple tools can be handled without searching for an empty bucket elsewhere in the table.

## 2. Pointer stability
I used std::unique_ptr<Tool> to manage the lifetime of each tool. The registry stores these pointers inside vectors, and resizing creates a new vector of buckets and moves the existing unique_ptr objects into their new buckets. Although the unique_ptr objects move, the actual Tool objects remain at their original memory addresses. This means a Tool* returned by find() remains valid after the registry resizes, as long as the tool itself has not been removed or destroyed.

I verified this with a test that saved the pointer to a tool named original, added 1,000 additional tools, and then checked that the pointer returned by find(“original”) was unchanged. The test passed, supporting the decision to keep the actual tools separately allocated rather than storing them directly inside the bucket vectors.


## 3. Calculator trace
For the expression 2 * (3 + 4) - 5, the calculator uses a values stack and an operators stack. The values stack stores numbers, while the operators stack stores operations that have not yet been applied.

First, the calculator reads 2 and pushes it onto the values stack. It then reads '*' and pushes multiplication onto the operators stack. When it encounters '(', it begins evaluating the expression inside the parentheses. It pushes 3 and then '+', followed by 4. When it reaches ')', it performs 3 + 4, leaving 7 as the result inside the parentheses.

The calculator then processes the multiplication. It applies 2 * 7, producing 14. Finally, it reads '-' and 5 and performs the subtraction, leaving 9 as the final result.

## 4. Errors as values
I used return values and error strings to report failures in the tools, Workspace::resolve(), and calc_evaluate(). This allows the caller to check whether an operation succeeded and decide what to do next. For example, resolve() returns false and provides a reason when a path is invalid, while the calculator reports division by zero when it encounters a zero denominator. I used return values and error messages so the program can tell the caller what went wrong without throwing an exception every time an operation fails.

An example where exceptions are appropriate is in my Conversation::at() function, which throws std::out_of_range when the requested index is outside the valid range. This is appropriate because the function is expected to return a reference, and it cannot return a valid element for an invalid index.


## 5. What I would design differently
If I did this project again, I would write my tests earlier than I did this time. That way, I would be able to ensure each component is properly functioning before moving on to the next. Writing the tests at the end made it more difficult to debug because it was hard to pin point exactly where my code was not functioning properly. 
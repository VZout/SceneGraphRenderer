# Capitalization

Names are capitalized in the same way as in most of the C++ standard.

* function parameters use snake_case style
* template parameters use **C**amel**C**ase style
* custom class names use **C**amel**C**ase style
* variable names use snake_case style
* macro and constant names use **ALL_CAPS** style

# Spacing and indentation

K&R indentation style is used (see K&R TBS).
Standard constructs, i.e. for, while, if, etc have a space between identifier and opening parentheses, e.g. 
for (...).
There is no space between function name and the parentheses, as well as between the parentheses and the content between them, e.g. fun(...).
There is no space between template name and < symbol, as well as between < and > symbols and the template parameters, e.g. tmp<...>.
Multiple function or template parameters are separated by space after the comma.
There is no space between reference and pointer (& and *) modifiers and the type name (e.g. int& b).
If the parameters of a function span several lines, the indentation of all parameters matches the opening parenthesis. The same goes for template parameters.

```cpp
#include <vector>
 
std::vector<int, MyAllocator> v;
 
int complex_function(int long_param_name,
                     int& another_param_name);
 
int main(int argc, char** argv)
{
    if (argc == 2) {
        v.push_back(23);
    }
}
```

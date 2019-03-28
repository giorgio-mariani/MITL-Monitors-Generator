# Bachelor's Thesis Project
## Project Statement
Given a MATLAB Simulink block diagram *D* and a temporal property *F*, the aim of this project is to develop methods and tools to automatically generate a Simulink block *M* which is a monitor for checking *F* within *D*. In other words, the project aims at producing a Property-to-Simulink compiler.

The logic used to specify these temporal properties is the *Metric Interval Temporal Logic* (MITL) [(O. Maler, 2004)](#references). MITL is a type of temporal logic in which properties are specified for bounded time intervals. It was inspired by the renowned *Linear Temporal Logic* (LTL), which is used in traditional formal verification methods.

## Usage
Invoke the command

	run.sh -d <output_dir> -f <formula_file>
(linux) or the command 

	run.bat -d <output_dir> -f <formula_file>
(windows) to run the system. This call generates the folder `<output_dir>` containing monitors for all the temporal formulas defined in `<formula_file>` (see [this](#formula-file-syntax) section for more info about the file syntax).
The generated directory `<output_dir>` will then contain a Simulink library that may be used for the property checking. More precisely, this library will contain one or more blocks, one per formula; adding these blocks to a model allows for the verification of the associated property.

In order to use the library the user should add the the folder `<output_dir>` to the matlab path, then using the **Simulink Library Browser** drag & drop the necessary block in the target model, and finally simulate the system. If the output signal of the block (at any given time instant during simulation) becomes `1`, then the MITL formula associated to the monitor block was not satisfied for at least an instant.

***Attention: Sometimes it is necessary to refresh the Simulink Library Browser in order to see the generated library. This can be done by pressing `F5` in the Library Browser.***

## Formula File Syntax
Described here is what format the formula file should comply. See [(O. Maler, 2004)](#references) to look at the semantics of MITL.

***Attention: The formula ﬁle encoding should be ISO/IEC 8859-1 (usually denoted as latin1). However, if such ﬁle does not contain any special character and has a one-byte ASCII based encoding, then the system should be able to correctly parse it.***

The file must contain a sequence of MITL formulas, interleaved by the character `|`. Each formula can also be preceded by a name (which can contain white spaces), the latter separated by the former using `:`.

#### Example:
	First Formula Name: <formula> | <formula> | Second Name:<formula>

### Formula Representation
In order to represent formulas, each MITL operator is mapped to a respective token (i.e. φ1 ∨ φ2 should be expressed as `<phi1> OR <phi2>`). This mapping is expressed as follows:

 * ¬φ → `NOT <phi>`
 * φ1 ∨ φ2 → `<phi1> OR <phi2>` 
 * φ1 ∧ φ2 → `<phi1> AND <phi2>` 
 * φ1 U[α,β] φ2 → `<phi1> UNTIL[α,β] <phi2>`
 * ◇[α,β] φ → `FUTURE[α,β] <phi>` 
 * □[α,β] φ → `GLOBALLY[α,β] <phi>`


#### Predicates 
Atomic MITL formulas take the form of linear constraints denoted as predicates. Specifically, a predicate ρ 

ρ = 5x + 6y + 40z ≥ 5

should be represented through the string `5*x + 6*y + 40*z >= 5`.
The available relations are: `=`, `∼=`, `<=`, `>=`, `<`, and `>`. If the predicate is a boolean constant then it can be simply expressed with keywords `TRUE` or `FALSE`.

***Note: Each of the keywords (TRUE, FALSE, NOT, OR, AND, UNTIL, GLOBALLY and FUTURE) is case insensitive. For example UNTIL is equivalent to Until, UnTiL, etc.***

#### Example of a formula file

	Formula 1: Globally[1,2](x<=0 And y>=0) Or y+2*x=1 Until[0,5] 5*z=50 
	| 
	FuTurE[5,10] position + velocity < 30 
	| 
	another formula: NOT v>=0.001 OR GLOBALLY[1e-2,0.1] p <= 2e5

### Operator Precedence and Associativity 
Operators precedence is described by:

|**Precedence**|         |
|---|--------------------|
|1  |NOT GLOBALLY FUTURE |
|2  |AND                 |
|3  |OR                  |
|4  |UNTIL               |

Operators on the same row have same precedence, while lower rows have lower precedence than higher rows. 

Associativity of all binary operators is always to the left.
As an example, the formula `(<phi1> OR <phi2> OR <phi3>)` is equivalent to the formula `((<phi1> OR <phi2>) OR <phi3>)`.


-----------------------------------------

# References
 [1] Oded Maler, Dejan Nickovic. Monitoring Temporal Properties of Continuous Signals. Lecture Notes in Computer Science, volume 3253, pages 152–166.
 
 [2] Paolo Zuliani, André Platzer, Edmund M. Clarke. Bayesian statistical model checking with application to Stateflow/Simulink verification. Formal Methods in System Design 43(2): 338-367 (2013).




$$
\begin{align}
[\text{Prog}] &\to [\text{Stamt}]^*
\\
[\text{Stamt}] &\to
\begin{cases}
    \text{exit}([\text{Expr}]); \\
    \text{assume}\space\text{iden} = [\text{Expr}]; \\
    \text{iden} = [\text{Expr}]; \\
    \text{incase}([\text{Expr}]) [\text{Scope}] [\text{Ifpred}]\\
    [\text{Scope}]
\end{cases}
\\
[\text{Ifpred}] &\to
\begin{cases}
    \text{elif}([\text{Expr}]) [\text{Scope}] [\text{Ifpred}]\\
    \text{else}[\text{Scope}] \\
    \epsilon \\
\end{cases}
\\
[\text{Scope}] &\to \{[\text{Stamt}]^*\} \\
[\text{Expr}] &\to 
\begin{cases}
[\text{Term}] \\
[\text{BinExpr}]
\end{cases} 
\\
[\text{BinExpr}] &\to
\begin{cases}
[\text{Expr}] + [\text{Expr}] & \text{prec = 0} \\
[\text{Expr}] - [\text{Expr}] & \text{prec = 1} \\
\end{cases}
\\
[\text{Term}] &\to
\begin{cases}
\text{int\_lit} \\
\text{iden} \\
([\text{Expr}])
\end{cases}
\end{align}
$$
-module(tak).
-export([tak/3]).
-export([tak/0]).

tak(X, Y, Z) when X =< Y ->
   Z;
tak(X, Y, Z) ->
   A1 = tak(X-1, Y, Z),
   A2 = tak(Y-1, Z, X),
   A3 = tak(Z-1, X, Y),
   tak(A1, A2, A3).

tak() -> 
   tak(18, 12, 6).

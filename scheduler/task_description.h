int max_time = 40;
tproc tasks[] = {
    //pid  //activation  //length  //remaining  //period
    {1,     0,          1,          1,          3},
    {2,     0,          1,          1,          5},
    {3,     1,          1,          1,          2},
};
/*

_______________________________________________________________________________________________________________________
Question: Proposez et testez un système de tâche qui est ordonnançable avec EDF et qui n’est pas ordonnançable avec RM.
-----------------------------------------------------------------------------------------------------------------------

Ce système de tâche marche avec EDF mais pas avec RM pour les raisons suivantes:
PROPRIETE:
    On a U ==> utilisation CPU
    Si U ≤ n.(2exp(1/n) − 1) alors il existe un ordonnancement satisfaisant toutes les échéances.

    Or:
    * U = 1/3 + 1/5 + 1/2 = 1.033
    * n.(2exp(1/n) − 1) = 0.779
     ____________
    | CONCLUSION |

    1.033 > 0.779 donc toutes les échéances en RM ne seront pas satisfaites
    On constate d'ailleurs sur le graphe d'exécution que la tâche T2 ne respecte plus ses deadlines en RM après 
    sa troisième échéance.

    - de plus U > 1

    - En plus, le théorème de la zone critique n'est pas respecté, les taches ne respectent pas leurs premières échéances
        et ne respecteront problablement pas les échéances futures. Le phénomène est observé pour T2 en RM après 
        sa troisième échéance.
 */
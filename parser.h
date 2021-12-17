#define SYNTAX_OK 0           //syntax ok
#define SYNTAX_ERROR 2        //chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu).
#define SEM_ERROR_DEFINE 3    //sémantická chyba v programu – nedefinovaná funkce/proměnná, pokus o redefinici proměnné, atp.
#define SEM_ERROR_INCOMP 4    //sémantická chyba v příkazu přiřazení (typová nekompatibilita).
#define SEM_ERROR_FUNCPARAM 5 //sémantická chyba v programu – špatný počet/typ parametrů či návratových hodnotu volání funkce či návratu z funkce.
#define SEM_ERROR_EXPRESS 6   //sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech
#define SEM_ERROR 7           //ostatní sémantické chyby.
#define NIL_ERROR 8          //běhová chyba při práci s neočekávanou hodnotou nil
#define ZERO_DIVISION_ERROR 9 //běhová chyba celočíselného dělení nulovou konstantou.
#define MEM_ALLOC_ERROR 99    //interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alokace paměti atd.)

int program();
void changeError(int n);
int tryGetToken();
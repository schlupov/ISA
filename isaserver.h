#ifndef ISASERVER_ISASERVER_H
#define ISASERVER_ISASERVER_H
#include <netdb.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <arpa/inet.h>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <regex>
#define MAXSIZEOFREQUEST 4096
#define SA struct sockaddr
#define NEWBOARD 0
#define UPGRADEBOARD 1
#define CONFLICT 409
#define NOTFOUND 404
#define OK 200
#define POST_OK 201
#define BADREQUEST 400
#include <utility>

/**
 * Pomocná struktura pro práci s nástěnkami.
 * Každá nástěnka představuje jednu takovouto strukturu,
 * má jméno a obsahuje příspěvky náležející dané nástěnce.
 */
struct Board {
    std::string boardStructName;
    std::vector<std::string> posts;
};

/**
 * Funkce tiskne nápovědu na standardní výstup.
 */
int help();

/**
 * Funkce vytváří soket a manipuluje s ním, aby byl připravený naslouchat na určitém portu.
 * V případě, kdy klient zašle nějaký požadavek zavolá tato funkce další funkci resolveCommand,
 * která se stará o následné zpracování požadavku. Když metoda resolveCommand vrátí číslo 201,
 * přidá tato funkce novou nástěnku do seznamu nástěnek.
 * @param port Port na kterém soket naslouchá, tato hodnota je dána argumentem programu.
 * @return Vrací 0  v případě, že vše proběhlo v pořádku.
 */
int startServer(int port);

/**
 * Funkce se stará o základní rozhodování, která http metoda byla v požadavku použita. V návaznosti na to
 * je pak zavolána příslušná funkce, která zpracuje obsah, který byl načtený ze soketu do bufferu o pevné délce.
 * @param connfd Soket jehož obsah se bude číst.
 * @param allBoards Seznam obsahující všechny nástěnky.
 * @param newBoard Nová nástěnka, která bude v případě metody POST vytvořena.
 * @return Vrací kód, který odpovídá návratovým kódům http požadavků.
 */
int resolveCommand(int connfd, std::list<Board>& allBoards, Board &newBoard);

/**
 * Funkce zkontroluje, že se skutečně jedná o post metodu a následně zkontroluje hlavičky.
 * Pokud je vše v pořádku, vytvoří se nová nástěnka a vrací se kód 201, pokud hlavičky nejsou v pořádku,
 * pak se vrací kód 404. Pokud jsou hlavičky v pořádku a nejedná se o požadavek na vytvoření nové
 * nástěnky, pak se vrací kód 1, který značí, že se má pozměnit už existující nástěnka.
 * @param buff Proměnná obsahuje požadavek od klienta.
 * @param newBoard Struktura pro novou nástěnku.
 * @param allBoards Seznam všech nástěnek.
 * @return Celé číslo indikující co za akci má být dále provedeno.
 */
int post(char *buff, Board &newBoard, std::list<Board> &allBoards);

/**
 * Pomocná funkce, která rozkouskuje řetězec podle zadaného oddělovače a jednotlivé části vloží
 * do vectoru.
 * @param str Řětězec, který se má rozdělit.
 * @param delimiter Oddělovač podle kterého bude řetězec rozdělen.
 * @param out Vektor obsahující jednotlivé části původního řetězce.
 */
void tokenize(std::string const &str, char delimiter, std::vector<std::string> &out);

/***
 * Funkce zjišťuje, jestli řetězec předaný jako parametr této funkci obsahuje pouze
 * platné ascii znaky.
 * @param s Řetězec u kterého kontrolujeme jednotlivé znaky.
 * @return Vrací true, pokud jsou všechny znaky v řetězci platné ascii znaky, jinak false.
 */
bool isASCII(const std::string& s);

/**
 * Funkce rozdělí požadavek od klienta a vrátí pouze tělo požadavku. Tato funkce počítá s tím,
 * že tělo je od hlaviček odděleno pomocí \r\n\r\n.
 * @param command Celý požadavek od klienta, hlavičky i tělo.
 * @return Vrací tělo požadavku od klienta.
 */
std::string getContent(const std::string &command);

/**
 * Funkce mění obsah specifikované nástěnky a patřičného příspěvku na nástěnce.
 * Funkce se zároveň stará o kontrolu hlaviček a požadavku od klienta.
 * V případě, kdy požadavek není korektně formulován nebo content-type není text/plain,
 * tak se vrací číslo 404. Pokud je content-length 0, pak se vrací číslo 400. Pokud je vše
 * v pořádku a příspěvek byl změněn, tak se vrací 201.
 * @param buff Proměnná obsahuje požadavek od klienta.
 * @param allBoards Seznam všech nástěnek.
 * @return Vrací číslo, které odpovídá http návratovému kódu, které se následně pošle klientovi.
 */
int upgradeBoardContent(char *buff, std::list<Board>& allBoards);

/**
 * Funkce rozdělí požadavek od klienta na části a ty pak vloží do vektoru, který funkce vrací.
 * Funkce nejdříve rozdělí požadavek podle mezer a následně podle lomítek.
 * @param command Požadavek od klienta.
 * @return Vektor obsahující řetězce z požadavku od klienta.
 */
std::vector<std::string> getCommandPartsAsVector(std::string &command);

/**
 * Funkce zpracovává http metodu get. Nejdříve se ujistí, že se skutečně jedná o GET metodu tím, že najde
 * GET v požadavku od klienta, následně se zkontroluje jestli se jedná o žádost o vypsání všech
 * nástěnek nebo jen obsahu jedné určité nástěnky. V obou případech dojde pomocí regulárního
 * výrazu  ke kontrole syntaxe požadavku. Před vytvořením a odesláním odpovědi klientovi se ještě najdou
 * všechny nežádoucí znaky jako \\n a zamění se za \n, aby pak klient skutečně tisknul nový řádek.
 * @param allBoards Seznam všech nástěnek.
 * @param connfd Soket přes který se odešle odpověď klientovi.
 * @param buff Požadavek od klienta.
 * @return Vrací číslo, které odpovídá návratovému kódu http odpovědi.
 */
int getInfo(std::list<Board> &allBoards, int connfd, char *buff);

/**
 * Funkce se stará o smazání pomocí delete. Nejdříve se ujistí, že se skutečně jedná o DELETE
 * metodu tím, že najde DELETE v požadavku od klienta. Následně dojde pomocí regulárního
 * výrazu  ke kontrole syntaxe požadavku a k rozhodnutí, jestli si klient přál smazat celou nástěnku nebo jen
 * její určitý příspěvek.
 * @param buff Požadavek od klienta.
 * @param allBoards Seznam všech nástěnek.
 * @return Vrací číslo, které odpovídá návratovému kódu http odpovědi.
 */
int deleteBoard(char *buff, std::list<Board>& allBoards);

/**
 * Funkce slepuje dohromady vektor, který obsahuje rozkouskované jednotlivé příspěvky pro danou nástěnku.
 * @param board Nástěnak jejíž obsah nás zajímá.
 * @return Vrací obsah nástěnky jako řetězec.
 */
std::string getContentOfPost(Board &board);

/**
 * Pomocná funkce v řetězci data nalezne řetězec toSearch a nahradí ho za řetězec replaceStr.
 * @param data Řetězec ve kterém se bude něco měnit.
 * @param toSearch Řetězec který se má vyhledat.
 * @param replaceStr Řetězec který nahrazuje původní řetězec.
 */
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);

/**
 * Funkce zjišťuje, jestli zadaný řetězec obsahuje znaky podle zadaného regulárního výrazu,
 * @param str Řetězec ve kterém se bude vyhledávat.
 * @param reg Regulární výraz aplikovaný na zadaný řetězec.
 * @return Vrací true v případě, kdy řetezec obsahuje podřetězec, který odpovídá zadanému regulárnímu výrazu.
 */
bool isMatch(std::string str, std::regex reg);

/**
 * Funkce se stará o změnu obsahu příspěvku pomocí put. Nejdříve se ujistí, že se skutečně jedná o PUT
 * metodu tím, že najde PUT v požadavku od klienta. Následně dojde pomocí regulárního
 * výrazu  ke kontrole syntaxe požadavku. Pokud je vše v pořádku a příspěvek s daným číslem skutenčě
 * již existuje, dojde ke změně obsahu příspěvku.
 * @param buff Požadavek od klienta.
 * @param allBoards Seznam všech nástěnek.
 * @return Vrací číslo, které odpovídá návratovému kódu http odpovědi.
 */
int updateSpecificPost(char *buff, std::list<Board> &allBoards);

/**
 * Funkce hledá pozici podřetězce method v řetězci putCommandParts.
 * @param putCommandParts Řetězec ve ktrerém se vyhledává.
 * @param method Metoda kterou se snažíme nejít.
 * @return Vrací pozici podřetězce method v řetězci putCommandParts.
 */
long getPosition(std::vector<std::string> &putCommandParts, const std::string &method);

/**
 * Funkce připravuje odpověď pro klienta na základě obdrženého čísla v parametru code.
 * @param code Číslo které určuje jaká http odpověď se bude vytvářet.
 * @return Vrací řetězec obsahující odpověď pro klienta.
 */
std::string prepareRespond(int code);
#endif //ISASERVER_ISASERVER_H

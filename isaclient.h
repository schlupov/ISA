#ifndef XCHLUP08_2_ISACLIENT_H
#define XCHLUP08_2_ISACLIENT_H
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <regex>
#include <sstream>
#include <sys/types.h>
#include <err.h>
#define SA struct sockaddr
#define MAXSIZEOFREQUEST 4096

/**
 * Enum pro konečný automat, který převádí
 * argumenty programu na http požadavek.
 */
typedef enum {
    start,
    board,
    item,
    add,
    delete_commmand,
    list,
    update,
    item_add,
    item_delete_command
} States;

/**
 * Funkce tiskne nápovědu na standardní výstup.
 */
int help();

/**
 * Funkce vytváří soket, který naslouchá na určitém portu a adrese.
 * @param port Port kam se soket připojí.
 * @param host Adresa kam se soket připojí.
 * @param command HTTP požadavek.
 * @param contentForPost Obsah těla požadavku.
 * @return Vrací číslo 0 v případě úspěchu, jinak -1.
 */
int connect(char *port, char *host, char *command, char *contentForPost);

/**
 * Funkce převádí vstupní argumenty programu na odpovídající http požadavek.
 * @param command Vstupní argumenty programu.
 * @param fullHttpCommand Celý http požadavek.
 * @return Vrací http požadavek.
 */
char *convertCommandtoHttpRequest(char *command, char *fullHttpCommand);

/**
 * Pouze pomocná funkce, poskládá vše z funkce convertCommandtoHttpRequest dohromady a vrací
 * výsledný http požadavek.
 * @param whichHttpCommand Určuje o jaký http požadavek půjde.
 * @param basicString Řetězec určující akci, která se bude provádět, např. delete.
 * @param fullHttpCommand Celý http požadavek.
 * @return Vrací http požadavek.
 */
char* createHttpCommand(std::string whichHttpCommand, const std::string& basicString, char *fullHttpCommand);

/**
 * Funkce vytvoří výsledný http požadavek, tj hlavičky i tělo požadavku, který se bude
 * odesílát serveru.
 * @param sockfd Otevřený soket.
 * @param method HTTP metoda, která se použije.
 * @param port Port kde server naslouchá.
 * @param host Adresa kam se klient připojí pro odeslání požadavku.
 * @param data Tělo požadatavku.
 * @return Vrací číslo 0 v případě úspěchu, jinak -1.
 */
int communicateWithServer(int sockfd, char *method, const char *port, const char *host, char *string);

/**
 * Funkce rozdělí odpověď od serveru a vrátí pouze tělo odpovědi. Tato funkce počítá s tím,
 * že tělo je od hlaviček odděleno pomocí \r\n\r\n.
 * @param command Celá odpověď od serveru, hlavičky i tělo.
 * @return Vrací tělo odpovědi.
 */
std::string getContent(const std::string &command);

/**
 * Funkce zjišťuje, jestli zadaný řetězec obsahuje znaky podle zadaného regulárního výrazu,
 * Tato funkce slouží k ověření správnosti zadaných vstupních argumentů programu.
 * @param str Řetězec ve kterém se bude vyhledávat.
 * @param reg Regulární výraz aplikovaný na zadaný řetězec.
 * @return Vrací true v případě, kdy řetezec obsahuje podřetězec, který odpovídá zadanému regulárnímu výrazu.
 */
bool isMatch(std::string str, std::regex reg);

/**
 * Funkce kontroluje, že byly zadány správné vstupní argumenty programu pomocí regulárních
 * výrazů. Regulární výrazy zároveň kontrolují, že se uživatel snaží vytvořit nástěnku se jménem
 * složeným ze znaků a-z nebo z čísel. Zároveň <content> může obsahovat pouze ascii znaky.
 * Funkce také kontroluje, že byl zadán správný počet argumentů programu.
 * @param command Vstupní argumenty programu.
 * @return Vrací true, pokud je vše v pořádku, jinak false.
 */
bool checkCommandLineArguments(const std::string &command, int argc);

/**
 * Funkce rozdělí odpověď od serveru a vrátí pouze hlavičky požadavku. Tato funkce počítá s tím,
 * že tělo je od hlaviček odděleno pomocí \r\n\r\n.
 * @param command Celá odpověď od serveru, hlavičky i tělo.
 * @return Vrací hlavičky odpovědi od serveru.
 */
std::string getHeaders(const std::string &command);

/**
 * Funkce hledá v hlavičkách odpovědi návratový kód od serveru.
 * @param headers Hlavičky odpovědi od serveru.
 * @return Vrací kód, který vrátil server klientovi.
 */
int checkHttpReturnCode(std::string headers);

/**
 * Pomocná funkce,která vyhledává v řetězci předem stanovená čísla.
 * @param headers Hlavičky odpovědi od serveru.
 * @param regex Regulární výraz určující, která čísla se hledají v headers.
 * @return Vrací číslo jako řetězec v případě úspěchu, jinak prázdný řetězec.
 */
std::string getCode(std::string headers, std::regex regex);
#endif

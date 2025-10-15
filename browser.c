/*DANET Tudor - 311CC*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Page
{
    int id;
    char url[50];
    char *description;
} Page;

typedef struct PageNode
{
    Page *page;
    struct PageNode *next;
    struct PageNode *prev;
} PageNode;

typedef struct Stack
{
    Page *page;
    struct Stack *next;
} Stack;

typedef struct Tab
{
    int id; // nr de tab-uri in cazul santinelei
    Page *currentPage;
    Stack *backStack;
    Stack *frontStack;
} Tab;

typedef struct TabNode
{
    Tab *tab; // int data
    struct TabNode *next;
    struct TabNode *prev;
} TabNode;

typedef struct Browser
{
    Tab *currentTab;
    TabNode *santinela;
} Browser;

// functii generale pentru stive:
Stack *pushStiva(Stack *stiva, Page *page)
{
    Stack *nod = (Stack *)malloc(sizeof(Stack));
    nod->page = page;
    nod->next = stiva;
    return nod;
}

Stack *popStiva(Stack *stiva)
{
    if (stiva == NULL)
    {
        return NULL;
    }
    Stack *temp = stiva;
    stiva = stiva->next;

    free(temp);
    return stiva;
}

void stergeStiva(Stack *stiva)
{
    while (stiva != NULL)
    {
        stiva = popStiva(stiva);
    }
}

void afisareStiva(Stack *stiva, FILE *out)
{
    if (stiva == NULL)
    {
        return;
    }

    Stack *current = stiva;
    while (current != NULL)
    {
        fprintf(out, "%s\n", current->page->url);
        current = current->next;
    }

    return;
}

Stack *inversareStiva(Stack *stiva)
{
    Stack *inversa = NULL;
    while (stiva != NULL)
    {
        inversa = pushStiva(inversa, stiva->page);
        stiva = popStiva(stiva);
    }
    return inversa;
}
// functii generale pentru stive:

// functii generale pentru pagini:
Page *paginaBasic()
{
    Page *page = (Page *)malloc(sizeof(Page));
    page->id = 0;
    strcpy(page->url, "https://acs.pub.ro/");

    // memorie fix cat pentru descrierea basic
    page->description = (char *)malloc(18 * sizeof(char));
    strcpy(page->description, "Computer Science\n");
    return page;
}

// initializarea listei de pagini
PageNode *initPageList()
{
    PageNode *start = (PageNode *)malloc(sizeof(PageNode));
    start->page = paginaBasic();
    start->next = start;
    start->prev = start;
    return start;
}

// adaugam o noua pagina in lista
PageNode *pushPagina(PageNode *start, Page *pag)
{
    PageNode *pageListElem = (PageNode *)malloc(sizeof(PageNode));
    pageListElem->page = pag;
    pageListElem->next = start;
    pageListElem->prev = start->prev;
    start->prev->next = pageListElem;
    start->prev = pageListElem;
    return start;
}

// completarea listei de pagini
PageNode *compPageList(FILE *file, PageNode *start)
{
    if (file == NULL)
    {
        return NULL;
    }

    int nr_pagini;
    if (fscanf(file, "%d\n", &nr_pagini) != 1)
    {
        return NULL;
    }

    if (nr_pagini <= 0)
    {
        return NULL;
    }

    // citim paginile
    for (int i = 1; i <= nr_pagini; i++)
    {
        Page *page = (Page *)malloc(sizeof(Page));

        fscanf(file, "%d\n", &page->id);

        fgets(page->url, sizeof(page->url), file);
        page->url[strcspn(page->url, "\n")] = '\0'; // scoatem newline-ul

        char buffer[256];
        fgets(buffer, sizeof(buffer), file);

        // pastram newline-ul in descriere
        page->description = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
        strcpy(page->description, buffer);

        start = pushPagina(start, page);
    }

    return start;
}
// functii generale pentru pagini:

// functii generale pentru tab-uri:
TabNode *gasesteCurent(Browser *browser)
{
    TabNode *current = browser->santinela->next;
    while (current != browser->santinela)
    {
        if (current->tab == browser->currentTab)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// initializarea santinelei
TabNode *initSantinela()
{
    TabNode *santinela = (TabNode *)malloc(sizeof(TabNode));
    santinela->tab = (Tab *)malloc(sizeof(Tab));
    // initializam id-ul santinelei pentru a retine in el cate tab-uri sunt deschise
    santinela->tab->id = -1;
    santinela->next = santinela;
    santinela->prev = santinela;
    return santinela;
}

int numarTaburiDeschise(Browser *browser)
{
    int count = 0;
    TabNode *current = browser->santinela->next;
    while (current != browser->santinela)
    {
        count++;
        current = current->next;
    }
    return count;
}
// functii generale pentru tab-uri:

// functiile cerute:
Browser *NEW_TAB(Browser *browser, PageNode *start)
{
    TabNode *tabNode = (TabNode *)malloc(sizeof(TabNode));
    tabNode->tab = (Tab *)malloc(sizeof(Tab));

    // actualizam nr de tab-uri
    browser->santinela->tab->id++;
    tabNode->tab->id = browser->santinela->tab->id;

    // pagina de baza
    tabNode->tab->currentPage = start->page;

    tabNode->tab->backStack = NULL;
    tabNode->tab->frontStack = NULL;

    // adaugam nodul cu noul tab in lista
    tabNode->next = browser->santinela;
    tabNode->prev = browser->santinela->prev;
    browser->santinela->prev->next = tabNode;
    browser->santinela->prev = tabNode;

    browser->currentTab = tabNode->tab;

    return browser;
}

Browser *CLOSE(Browser *browser, FILE *out)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);
    if (current == NULL || current == browser->santinela->next)
    {
        fprintf(out, "403 Forbidden\n");
        return browser;
    }

    // eliberam memoria pentru tab-ul inchis
    stergeStiva(current->tab->backStack);
    stergeStiva(current->tab->frontStack);
    free(current->tab);

    // actualizam lista
    current->prev->next = current->next;
    current->next->prev = current->prev;

    browser->currentTab = current->prev->tab;
    free(current);

    return browser;
}

Browser *OPEN(Browser *browser, int id, FILE *out)
{
    TabNode *current = browser->santinela->next;
    while (current != browser->santinela)
    {
        if (current->tab->id == id)
        {
            browser->currentTab = current->tab;
            return browser;
        }
        current = current->next;
    }
    fprintf(out, "403 Forbidden\n");
    return browser;
}

Browser *NEXT(Browser *browser)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);

    if (current->next != browser->santinela)
    {
        browser->currentTab = current->next->tab;
    }
    else
    {
        browser->currentTab = browser->santinela->next->tab;
    }
    return browser;
}

Browser *PREV(Browser *browser)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);

    if (current->prev != browser->santinela)
    {
        browser->currentTab = current->prev->tab;
    }
    else
    {
        browser->currentTab = browser->santinela->prev->tab;
    }
    return browser;
}

Browser *PAGE(Browser *browser, int id, PageNode *start, FILE *out)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);
    if (current == browser->santinela)
    {
        return browser;
    }

    if (start == NULL)
    {
        fprintf(out, "403 Forbidden\n");
        return browser;
    }

    PageNode *temp = start->next;
    while (temp != start)
    {
        if (temp->page->id == id)
        {
            // actualizam stiva backStack
            current->tab->backStack = pushStiva(current->tab->backStack, current->tab->currentPage);

            current->tab->currentPage = temp->page;

            if (current->tab->frontStack != NULL)
            {
                stergeStiva(current->tab->frontStack);
                current->tab->frontStack = NULL;
            }

            return browser;
        }
        temp = temp->next;
    }

    fprintf(out, "403 Forbidden\n");
    return browser;
}

Browser *BACKWARD(Browser *browser, FILE *out)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);

    if (current == NULL || current->tab->backStack == NULL)
    {
        fprintf(out, "403 Forbidden\n");
        return browser;
    }

    // ajustam stivele pentru noua pagina
    current->tab->frontStack = pushStiva(current->tab->frontStack, current->tab->currentPage);
    current->tab->currentPage = current->tab->backStack->page;
    current->tab->backStack = popStiva(current->tab->backStack);

    return browser;
}

Browser *FORWARD(Browser *browser, FILE *out)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);

    if (current == NULL || current->tab->frontStack == NULL)
    {
        fprintf(out, "403 Forbidden\n");
        return browser;
    }

    // ajustam stivele pentru noua pagina
    current->tab->backStack = pushStiva(current->tab->backStack, current->tab->currentPage);
    current->tab->currentPage = current->tab->frontStack->page;
    current->tab->frontStack = popStiva(current->tab->frontStack);

    return browser;
}

void PRINT(Browser *browser, FILE *out)
{
    // gasim nodul din lista corespunzator tab-ului curent
    TabNode *current = gasesteCurent(browser);

    // afisam id-ul fiecarei pagini deschise
    int deschise = numarTaburiDeschise(browser);
    while (deschise > 1)
    {
        fprintf(out, "%d ", current->tab->id);

        if (current->next != browser->santinela)
        {
            current = current->next;
        }
        else
        {
            current = browser->santinela->next;
        }
        deschise--;
    }
    fprintf(out, "%d", current->tab->id);

    // descrierea paginii curente
    if (current->next != browser->santinela)
    {
        current = current->next;
    }
    else
    {
        current = browser->santinela->next;
    }
    fprintf(out, "\n%s", current->tab->currentPage->description);
}

void PRINT_HISTORY(Browser *browser, int id, FILE *out)
{
    TabNode *current = browser->santinela->next;

    if (id > browser->santinela->tab->id || id < 0)
    {
        fprintf(out, "403 Forbidden\n");
        return;
    }

    // cautam pagina cu id-ul cerut
    while (current != browser->santinela)
    {
        if (current->tab->id == id)
        {
            break;
        }
        current = current->next;
    }

    if (current->tab->id != id)
    {
        fprintf(out, "403 Forbidden\n");
        return;
    }

    // inversam stiva frontStack pentru a afisa mai usor
    Stack *inversa = inversareStiva(current->tab->frontStack);
    afisareStiva(inversa, out);

    // pagina curenta
    fprintf(out, "%s\n", current->tab->currentPage->url);

    afisareStiva(current->tab->backStack, out);
}
// functiile cerute

// eliberari de memorie
void stergePagini(Browser *browser, PageNode *start)
{
    if (start == NULL)
    {
        return;
    }

    PageNode *current = start->next;
    while (current != start)
    {
        PageNode *temp = current;
        current = current->next;

        // eliberam pagina curenta
        free(temp->page->description);
        free(temp->page);
        free(temp);
    }

    free(start->page->description);
    free(start->page);
    free(start);
}

void stergeTaburi(Browser *browser)
{
    TabNode *current = browser->santinela->next;
    while (current != browser->santinela)
    {
        TabNode *temp = current;
        current = current->next;

        stergeStiva(temp->tab->backStack);

        stergeStiva(temp->tab->frontStack);

        free(temp->tab);
        free(temp);
    }

    free(browser->santinela->tab);
    free(browser->santinela);
    free(browser);
}
// eliberari de memorie

void ruleaza(char *linie, Browser **browser, PageNode *start, FILE *out)
{
    int id;

    // daca linia e goala ne oprim
    if (linie == NULL || strlen(linie) == 0)
    {
        return;
    }

    if (strstr(linie, "NEW_TAB") != 0)
    {
        *browser = NEW_TAB(*browser, start);
    }
    else if (strstr(linie, "CLOSE") != 0)
    {
        *browser = CLOSE(*browser, out);
    }
    else if (strstr(linie, "OPEN") != 0)
    {
        sscanf(linie + 5, "%d", &id);
        *browser = OPEN(*browser, id, out);
    }
    else if (strstr(linie, "NEXT") != 0)
    {
        *browser = NEXT(*browser);
    }
    else if (strstr(linie, "PREV") != 0)
    {
        *browser = PREV(*browser);
    }
    else if (strstr(linie, "PAGE") != 0)
    {
        int id;
        sscanf(linie + 5, "%d", &id);
        *browser = PAGE(*browser, id, start, out);
    }
    else if (strstr(linie, "BACKWARD") != 0)
    {
        *browser = BACKWARD(*browser, out);
    }
    else if (strstr(linie, "FORWARD") != 0)
    {
        *browser = FORWARD(*browser, out);
    }
    else if (strstr(linie, "PRINT_HISTORY") != 0)
    {
        int id;
        sscanf(linie + 14, "%d", &id);
        PRINT_HISTORY(*browser, id, out);
    }
    else if (strstr(linie, "PRINT") != 0)
    {
        PRINT(*browser, out);
    }
}

void citireFunctii(char *file1, char *file2, Browser **browser, PageNode *start)
{
    int nr_functii, i;
    FILE *file = fopen(file1, "r");
    FILE *out = fopen(file2, "w");

    // completarea listei de pagini
    start->prev = compPageList(file, start);

    fscanf(file, "%d", &nr_functii);
    fgetc(file);
    if (nr_functii <= 0)
    {
        fclose(file);
        return;
    }

    // rularea fiecarei functii din input
    char line[256];
    for (i = 0; i < nr_functii; i++)
    {
        fgets(line, 256, file);
        line[strcspn(line, "\n")] = '\0';

        ruleaza(line, browser, start, out);
    }

    stergeTaburi(*browser);
    stergePagini(*browser, start);

    fclose(file);
    fclose(out);
}

int main()
{
    // initializare browser, lista de pagini si tab 0
    Browser *browser = (Browser *)malloc(sizeof(Browser));
    browser->santinela = initSantinela();

    PageNode *start = initPageList();
    browser = NEW_TAB(browser, start);

    char *in, *out;
    in = (char *)malloc(50 * sizeof(char));
    strcpy(in, "tema1.in");
    out = (char *)malloc(50 * sizeof(char));
    strcpy(out, "tema1.out");

    citireFunctii(in, out, &browser, start);

    // eliberari nume file-uri
    free(in);
    free(out);
}

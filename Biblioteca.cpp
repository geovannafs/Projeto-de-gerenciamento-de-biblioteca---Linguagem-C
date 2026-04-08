//Importanto bibliotecas
#include <stdio.h>    // biblioteca para funções de entrada e saida
#include <stdlib.h>   // biblioteca para funções gerais 
#include <string.h>   // biblioteca para manipulação de strings
#include <time.h>     // biblioteca para manipulação de datas de emprestimos/cadastro
#include <ctype.h>    // biblioteca para usar tolower (conversão de caracteres), para poder rodar em diversos compiladores

// --- CONSTANTES ---
#define MAX_LIVROS 100
#define MAX_USUARIOS 100
#define MAX_EMPRESTIMOS 200

// --- PARTE 1: ESTRUTURAS DE DADOS E ENUMS ---

// Enumeração para o status do livro
typedef enum {
    DISPONIVEL = 1,
    EMPRESTADO = 2
} StatusLivro;

// Estrutura para Data
typedef struct {
    int dia;
    int mes;
    int ano;
} Data;

// Estrutura para Livro
typedef struct {
    int codigo;
    char titulo[101]; //Deixando 101 para guardar 100 caracteres mais o \0
    char autor[81];  //Deixando 81 para guardar 80 caracteres mais o \0
    char editora[61]; //Deixando 61 para guardar 60 caracteres mais o \0
    int ano_publicacao;
    int num_exemplares; // Quantidade total de exemplares
    int exemplares_disponiveis; // Quantidade que está na prateleira
    StatusLivro status; // Status geral 
} Livro;

// Estrutura para Usuário
typedef struct {
    int matricula;
    char nome_completo[101]; //Deixando 101 para guardar 100 caracteres mais o \0
    char curso[51]; //Deixando 51 para guardar 50 caracteres mais o \0
    char telefone[16]; //Deixando 16 para guardar 15 caracteres mais o \0
    Data data_cadastro; //Tipo de dados sendo a struct Data declarada anteriormente
} Usuario;

// Enumeração para o status do empréstimo
typedef enum {
    ATIVO = 1,
    DEVOLVIDO = 2,
    ATRASADO = 3
} StatusEmprestimo;

// Estrutura para Empréstimo
typedef struct {
    int codigo;
    int matricula_usuario;
    int codigo_livro;
    Data data_emprestimo; //Tipo de dados sendo a struct Data declarada anteriormente
    Data data_devolucao_prevista; //Tipo de dados sendo a struct Data declarada anteriormente
    StatusEmprestimo status; //Tipo de dados sendo a struct StatusEmprestimo declarada anteriormente
} Emprestimo;

// --- VETORES DE STRUCTS GLOBAIS ---
Livro livros[MAX_LIVROS]; //constante declarada anteriormente
Usuario usuarios[MAX_USUARIOS]; //constante declarada anteriormente
Emprestimo emprestimos[MAX_EMPRESTIMOS]; //constante declarada anteriormente

//inicializando as variaveis 
int num_livros = 0;
int num_usuarios = 0;
int num_emprestimos = 0;

// --- FUNÇÕES DE UTILDADE ---

// Função auxiliar para limpar o buffer de entrada e não dar erros nas entradas pelo teclado
void LimparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Função para converter string para minúsculas, substituindo a função strlwr por questões de compatibilidade
char *strlwr_custom(char *str) {
    for (char *p = str; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    return str;
}

// Função para gerar a data atual do sistema
Data GerarDataAtual() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    Data hoje;
    hoje.dia = tm_info->tm_mday;
    hoje.mes = tm_info->tm_mon + 1; // tm_mon é 0-11
    hoje.ano = tm_info->tm_year + 1900; // tm_year é anos desde 1900
    return hoje;
}

// Compara duas datas: 1 se data1 > data2, -1 se data1 < data2, 0 se iguais
int CompararDatas(Data d1, Data d2) {
    if (d1.ano != d2.ano) return (d1.ano > d2.ano) ? 1 : -1;
    if (d1.mes != d2.mes) return (d1.mes > d2.mes) ? 1 : -1;
    if (d1.dia != d2.dia) return (d1.dia > d2.dia) ? 1 : -1;
    return 0; // Datas iguais
}

// Calcula a data de devolução (7 dias após)
Data CalcularDataDevolucao(Data data_emprestimo) {
    struct tm tm_struct;
    time_t t;
    
    // Inicializa a estrutura tm
    tm_struct.tm_year = data_emprestimo.ano - 1900;
    tm_struct.tm_mon = data_emprestimo.mes - 1;
    tm_struct.tm_mday = data_emprestimo.dia;
    tm_struct.tm_hour = 0;
    tm_struct.tm_min = 0;
    tm_struct.tm_sec = 0;
    tm_struct.tm_isdst = -1; // Deixa o sistema determinar o DST

    // Converte tm para time_t
    t = mktime(&tm_struct);

    // Adiciona 7 dias (7 dias * 24 horas * 60 minutos * 60 segundos)
    t += 7 * 24 * 60 * 60;

    // Converte de volta para struct tm e depois para Data
    struct tm *info = localtime(&t);
    Data data_prevista;
    data_prevista.dia = info->tm_mday;
    data_prevista.mes = info->tm_mon + 1;
    data_prevista.ano = info->tm_year + 1900;

    return data_prevista;
}

// Busca o índice de um livro pelo código
int BuscarLivroPorCodigo(int codigo) {
    for (int i = 0; i < num_livros; i++) {
        if (livros[i].codigo == codigo) {
            return i;
        }
    }
    return -1;
}

// Busca o índice de um usuário pela matrícula
int BuscarUsuarioPorMatricula(int matricula) {
    for (int i = 0; i < num_usuarios; i++) {
        if (usuarios[i].matricula == matricula) {
            return i;
        }
    }
    return -1;
}

// --- PARTE 4: MANIPULAÇÃO DE ARQUIVOS (Salvar e Carregar) ---

// Protótipo para uso no SalvarDados
void AtualizarStatusEmprestimos();

// Salva todos os dados nos respectivos arquivos
void SalvarDados() {
    FILE *f_livros, *f_usuarios, *f_emprestimos;

    // 1. Livros
    f_livros = fopen("livros.txt", "w");
    if (f_livros == NULL) {
        printf("ERRO: Nao foi possivel abrir livros.txt para escrita.\n");
        return;
    }
    fprintf(f_livros, "%d\n", num_livros);
    for (int i = 0; i < num_livros; i++) {
        fprintf(f_livros, "%d|%s|%s|%s|%d|%d|%d|%d\n",
                livros[i].codigo, livros[i].titulo, livros[i].autor, livros[i].editora,
                livros[i].ano_publicacao, livros[i].num_exemplares, 
                livros[i].exemplares_disponiveis, livros[i].status);
    }
    fclose(f_livros);

    // 2. Usuários
    f_usuarios = fopen("usuarios.txt", "w");
    if (f_usuarios == NULL) {
        printf("ERRO: Nao foi possivel abrir usuarios.txt para escrita.\n");
        return;
    }
    fprintf(f_usuarios, "%d\n", num_usuarios);
    for (int i = 0; i < num_usuarios; i++) {
        fprintf(f_usuarios, "%d|%s|%s|%s|%d/%d/%d\n",
                usuarios[i].matricula, usuarios[i].nome_completo, usuarios[i].curso,
                usuarios[i].telefone, usuarios[i].data_cadastro.dia, 
                usuarios[i].data_cadastro.mes, usuarios[i].data_cadastro.ano);
    }
    fclose(f_usuarios);

    // 3. Empréstimos
    AtualizarStatusEmprestimos(); // Garante que o status atrasado está correto antes de salvar
    f_emprestimos = fopen("emprestimos.txt", "w");
    if (f_emprestimos == NULL) {
        printf("ERRO: Nao foi possivel abrir emprestimos.txt para escrita.\n");
        return;
    }
    fprintf(f_emprestimos, "%d\n", num_emprestimos);
    for (int i = 0; i < num_emprestimos; i++) {
        fprintf(f_emprestimos, "%d|%d|%d|%d/%d/%d|%d/%d/%d|%d\n",
                emprestimos[i].codigo, emprestimos[i].matricula_usuario, emprestimos[i].codigo_livro,
                emprestimos[i].data_emprestimo.dia, emprestimos[i].data_emprestimo.mes, 
                emprestimos[i].data_emprestimo.ano,
                emprestimos[i].data_devolucao_prevista.dia, emprestimos[i].data_devolucao_prevista.mes, 
                emprestimos[i].data_devolucao_prevista.ano,
                emprestimos[i].status);
    }
    fclose(f_emprestimos);

    printf("\n[Dados salvos automaticamente com sucesso.]\n");
}

// Carrega todos os dados dos arquivos na inicialização
void CarregarDados() {
    FILE *f_livros, *f_usuarios, *f_emprestimos;
    
    // 1. Livros
    f_livros = fopen("livros.txt", "r");
    if (f_livros != NULL) {
        fscanf(f_livros, "%d\n", &num_livros);
        for (int i = 0; i < num_livros; i++) {
            fscanf(f_livros, "%d|%99[^|]|%79[^|]|%59[^|]|%d|%d|%d|%d\n",
                   &livros[i].codigo, livros[i].titulo, livros[i].autor, livros[i].editora,
                   &livros[i].ano_publicacao, &livros[i].num_exemplares, 
                   &livros[i].exemplares_disponiveis, (int*)&livros[i].status);
        }
        fclose(f_livros);
    }

    // 2. Usuários
    f_usuarios = fopen("usuarios.txt", "r");
    if (f_usuarios != NULL) {
        fscanf(f_usuarios, "%d\n", &num_usuarios);
        for (int i = 0; i < num_usuarios; i++) {
            fscanf(f_usuarios, "%d|%99[^|]|%49[^|]|%14[^|]|%d/%d/%d\n",
                   &usuarios[i].matricula, usuarios[i].nome_completo, usuarios[i].curso,
                   usuarios[i].telefone, &usuarios[i].data_cadastro.dia, 
                   &usuarios[i].data_cadastro.mes, &usuarios[i].data_cadastro.ano);
        }
        fclose(f_usuarios);
    }

    // 3. Empréstimos
    f_emprestimos = fopen("emprestimos.txt", "r");
    if (f_emprestimos != NULL) {
        fscanf(f_emprestimos, "%d\n", &num_emprestimos);
        for (int i = 0; i < num_emprestimos; i++) {
            fscanf(f_emprestimos, "%d|%d|%d|%d/%d/%d|%d/%d/%d|%d\n",
                   &emprestimos[i].codigo, &emprestimos[i].matricula_usuario, &emprestimos[i].codigo_livro,
                   &emprestimos[i].data_emprestimo.dia, &emprestimos[i].data_emprestimo.mes, 
                   &emprestimos[i].data_emprestimo.ano,
                   &emprestimos[i].data_devolucao_prevista.dia, &emprestimos[i].data_devolucao_prevista.mes, 
                   &emprestimos[i].data_devolucao_prevista.ano,
                   (int*)&emprestimos[i].status);
        }
        fclose(f_emprestimos);
    }
    printf("[Dados de Livros, Usuarios e Emprestimos carregados com sucesso.]\n");
}

// Função de inicialização (Requisito 1 - Inicializar estruturas)
void InicializarEstruturas() {
    num_livros = 0;
    num_usuarios = 0;
    num_emprestimos = 0;
}


// --- PARTE 3: FUNÇÕES MODULARES (Cadastro e Busca) ---

// 1. Cadastro de Livros
void CadastrarLivro() {
    if (num_livros >= MAX_LIVROS) {
        printf("\nERRO: Limite de cadastro de livros atingido.\n");
        return;
    }

    Livro novo;
    int codigo_temp;

    printf("\n----- CADASTRO DE LIVRO -----\n");
    
    // Validação de Código
    do {
        printf("Codigo do livro (inteiro e unico): ");
        if (scanf("%d", &codigo_temp) != 1) {
            printf("Entrada invalida. Digite um numero.\n");
            LimparBuffer();
            codigo_temp = -1; // Força a repetição
        } else {
            LimparBuffer();
            if (BuscarLivroPorCodigo(codigo_temp) != -1) {
                printf("ERRO: Codigo ja existe. Escolha outro.\n");
                codigo_temp = -1;
            }
        }
    } while (codigo_temp <= 0 || codigo_temp == -1);

    novo.codigo = codigo_temp;

    printf("Titulo: ");
    scanf("%99[^\n]", novo.titulo);
    LimparBuffer();
    
    printf("Autor: ");
    scanf("%79[^\n]", novo.autor);
    LimparBuffer();
    
    printf("Editora: ");
    scanf("%59[^\n]", novo.editora);
    LimparBuffer();
    
    do {
        printf("Ano de publicacao: ");
        if (scanf("%d", &novo.ano_publicacao) != 1) {
            printf("Entrada invalida. Digite um numero.\n");
            LimparBuffer();
            novo.ano_publicacao = 0;
        } else {
            LimparBuffer();
        }
    } while (novo.ano_publicacao < 1000 || novo.ano_publicacao > 2099);

    do {
        printf("Numero de exemplares: ");
        if (scanf("%d", &novo.num_exemplares) != 1 || novo.num_exemplares <= 0) {
            printf("Entrada invalida. Digite um numero positivo.\n");
            LimparBuffer();
            novo.num_exemplares = 0;
        } else {
            LimparBuffer();
        }
    } while (novo.num_exemplares <= 0);

    // Inicializa exemplares disponiveis e status
    novo.exemplares_disponiveis = novo.num_exemplares;
    novo.status = DISPONIVEL;

    livros[num_livros] = novo;
    num_livros++;
    
    printf("\n? Livro cadastrado com sucesso! Disponiveis: %d\n", novo.exemplares_disponiveis);
    SalvarDados();
}

// 2. Cadastro de Usuários
void CadastrarUsuario() {
    if (num_usuarios >= MAX_USUARIOS) {
        printf("\nERRO: Limite de cadastro de usuarios atingido.\n");
        return;
    }

    Usuario novo;
    int matricula_temp;

    printf("\n----- CADASTRO DE USUARIO -----\n");
    
    // Validação de Matrícula
    do {
        printf("Matricula: ");
        if (scanf("%d", &matricula_temp) != 1) {
            printf("Entrada invalida. Digite um numero.\n");
            LimparBuffer();
            matricula_temp = -1; // Força a repetição
        } else {
            LimparBuffer();
            if (BuscarUsuarioPorMatricula(matricula_temp) != -1) {
                printf("ERRO: Matricula ja existe. Escolha outra.\n");
                matricula_temp = -1;
            }
        }
    } while (matricula_temp <= 0 || matricula_temp == -1);

    novo.matricula = matricula_temp;

    printf("Nome completo: ");
    scanf("%99[^\n]", novo.nome_completo);
    LimparBuffer();
    
    printf("Curso: ");
    scanf("%49[^\n]", novo.curso);
    LimparBuffer();
    
    printf("Telefone: ");
    scanf("%14[^\n]", novo.telefone);
    LimparBuffer();
    
    novo.data_cadastro = GerarDataAtual();

    usuarios[num_usuarios] = novo;
    num_usuarios++;
    
    printf("\n? Usuario cadastrado com sucesso! Data de cadastro: %d/%d/%d\n", 
           novo.data_cadastro.dia, novo.data_cadastro.mes, novo.data_cadastro.ano);
    SalvarDados();
}

// 3. Sistema de Empréstimos
void RealizarEmprestimo() {
    if (num_emprestimos >= MAX_EMPRESTIMOS) {
        printf("\nERRO: Limite de cadastro de emprestimos atingido.\n");
        return;
    }

    int codigo_livro, matricula_usuario;
    int idx_livro, idx_usuario;

    printf("\n----- REALIZAR EMPRESTIMO -----\n");

    // 1. Coleta e validação da Matrícula do Usuário
    do {
        printf("Matricula do Usuario: ");
        if (scanf("%d", &matricula_usuario) != 1) {
            printf("Entrada invalida.\n");
            LimparBuffer();
            matricula_usuario = -1;
            continue;
        }
        LimparBuffer();
        idx_usuario = BuscarUsuarioPorMatricula(matricula_usuario);
        if (idx_usuario == -1) {
            printf("ERRO: Usuario com matricula %d nao encontrado.\n", matricula_usuario);
            matricula_usuario = -1;
        }
    } while (matricula_usuario == -1);

    // 2. Coleta e validação do Código do Livro
    do {
        printf("Codigo do Livro: ");
        if (scanf("%d", &codigo_livro) != 1) {
            printf("Entrada invalida.\n");
            LimparBuffer();
            codigo_livro = -1;
            continue;
        }
        LimparBuffer();
        idx_livro = BuscarLivroPorCodigo(codigo_livro);
        if (idx_livro == -1) {
            printf("ERRO: Livro com codigo %d nao encontrado.\n", codigo_livro);
            codigo_livro = -1;
            continue;
        }
        if (livros[idx_livro].exemplares_disponiveis <= 0) {
            printf("ERRO: Livro '%s' indisponivel no momento (0 exemplares).\n", livros[idx_livro].titulo);
            codigo_livro = -1;
        }
    } while (codigo_livro == -1);

    // 3. Processa o Empréstimo
    Emprestimo novo;
    novo.codigo = (num_emprestimos > 0) ? emprestimos[num_emprestimos - 1].codigo + 1 : 1;
    novo.matricula_usuario = matricula_usuario;
    novo.codigo_livro = codigo_livro;
    novo.data_emprestimo = GerarDataAtual();
    novo.data_devolucao_prevista = CalcularDataDevolucao(novo.data_emprestimo);
    novo.status = ATIVO;

    // 4. Atualiza o Acervo
    livros[idx_livro].exemplares_disponiveis--;
    if (livros[idx_livro].exemplares_disponiveis == 0) {
        livros[idx_livro].status = EMPRESTADO;
    }

    // 5. Adiciona o Empréstimo
    emprestimos[num_emprestimos] = novo;
    num_emprestimos++;

    printf("\n? Emprestimo #%d realizado com sucesso!\n", novo.codigo);
    printf("    Livro: %s\n", livros[idx_livro].titulo);
    printf("    Usuario: %s\n", usuarios[idx_usuario].nome_completo);
    printf("    Devolucao prevista: %d/%d/%d\n", novo.data_devolucao_prevista.dia, 
           novo.data_devolucao_prevista.mes, novo.data_devolucao_prevista.ano);
    printf("    Exemplares disponiveis restantes: %d\n", livros[idx_livro].exemplares_disponiveis);
    SalvarDados();
}

// 4. Função para realizar devolução
void RealizarDevolucao() {
    int codigo_emprestimo, idx_emprestimo = -1;
    int codigo_livro, idx_livro;

    printf("\n----- REALIZAR DEVOLUCAO -----\n");

    if (num_emprestimos == 0) {
        printf("ERRO: Nao ha emprestimos registrados.\n");
        return;
    }

    // 1. Coleta e validação do Código do Empréstimo
    printf("Codigo do Emprestimo a ser devolvido: ");
    if (scanf("%d", &codigo_emprestimo) != 1) {
        printf("Entrada invalida.\n");
        LimparBuffer();
        return;
    }
    LimparBuffer();

    // Busca o empréstimo
    for (int i = 0; i < num_emprestimos; i++) {
        if (emprestimos[i].codigo == codigo_emprestimo) {
            idx_emprestimo = i;
            break;
        }
    }

    if (idx_emprestimo == -1) {
        printf("ERRO: Emprestimo #%d nao encontrado.\n", codigo_emprestimo);
        return;
    }

    // 2. Verifica status do empréstimo
    if (emprestimos[idx_emprestimo].status == DEVOLVIDO) {
        printf("ERRO: Emprestimo #%d ja foi devolvido anteriormente.\n", codigo_emprestimo);
        return;
    }

    // 3. Processa a Devolução
    codigo_livro = emprestimos[idx_emprestimo].codigo_livro;
    idx_livro = BuscarLivroPorCodigo(codigo_livro);

    // Atualiza status do empréstimo
    emprestimos[idx_emprestimo].status = DEVOLVIDO;

    // Atualiza o acervo (Aumenta o contador de exemplares disponíveis)
    if (idx_livro != -1) {
        livros[idx_livro].exemplares_disponiveis++;
        // Se o livro tinha 0 exemplares disponíveis e agora tem 1, atualiza o status geral
        if (livros[idx_livro].exemplares_disponiveis > 0) {
            livros[idx_livro].status = DISPONIVEL;
        }
        printf("\n? Devolucao do livro '%s' realizada com sucesso!\n", livros[idx_livro].titulo);
        printf("    Exemplares disponiveis atualizados: %d\n", livros[idx_livro].exemplares_disponiveis);
    } else {
        printf("\n? Devolucao de Emprestimo #%d registrada, mas o livro original nao foi encontrado no acervo.\n", codigo_emprestimo);
    }
    
    SalvarDados();
}

// 5. Função para pesquisar livros (por código, título ou autor)
void PesquisarLivros() {
    int opcao;
    char termo_busca[101]; // O termo de busca é o maior que pode ser um título
    int encontrados = 0;
    
    printf("\n----- PESQUISAR LIVROS -----\n");
    printf("1. Por Codigo\n");
    printf("2. Por Titulo\n");
    printf("3. Por Autor\n");
    printf("Escolha a opcao: ");
    if (scanf("%d", &opcao) != 1) {
        LimparBuffer();
        printf("Opcao invalida.\n");
        return;
    }
    LimparBuffer();

    if (opcao < 1 || opcao > 3) {
        printf("Opcao invalida.\n");
        return;
    }

    printf("Digite o termo de busca: ");
    scanf("%100[^\n]", termo_busca);
    LimparBuffer();
    
    // NOVO: Preparação para busca case-insensitive
    char termo_busca_lower[101];
    char campo_livro_lower[101];
    
    // Faz a cópia do termo de busca e converte para minúsculas uma vez
    strncpy(termo_busca_lower, termo_busca, 100);
    termo_busca_lower[100] = '\0';
    strlwr_custom(termo_busca_lower);


    printf("\n--- RESULTADOS DA PESQUISA ---\n");
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("| CODIGO | TITULO %-35s | AUTOR %-25s | EXEMPLARES | STATUS  |\n", "", "");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_livros; i++) {
        int match = 0;
        switch (opcao) {
            case 1: // Por Código
                if (atoi(termo_busca) == livros[i].codigo) match = 1;
                break;
            case 2: // Por Título (case-insensitive)
                // Copia o título do livro, converte para minúsculas e busca
                strncpy(campo_livro_lower, livros[i].titulo, 100);
                campo_livro_lower[100] = '\0';
                strlwr_custom(campo_livro_lower);
                if (strstr(campo_livro_lower, termo_busca_lower) != NULL) match = 1;
                break;
            case 3: // Por Autor (case-insensitive)
                // Copia o autor do livro, converte para minúsculas e busca
                strncpy(campo_livro_lower, livros[i].autor, 80);
                campo_livro_lower[80] = '\0';
                strlwr_custom(campo_livro_lower);
                if (strstr(campo_livro_lower, termo_busca_lower) != NULL) match = 1;
                break;
        }

        if (match) {
            printf("| %-6d | %-42s | %-31s | %-10d | %-7s |\n",
                   livros[i].codigo, livros[i].titulo, livros[i].autor, 
                   livros[i].exemplares_disponiveis, 
                   (livros[i].status == DISPONIVEL) ? "Disp." : "Emp.");
            encontrados++;
        }
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("Total de livros encontrados: %d\n", encontrados);
}

// 6. Função para pesquisar usuários (por matrícula ou nome)
void PesquisarUsuarios() {
    int opcao;
    char termo_busca[101];
    int encontrados = 0;
    
    printf("\n----- PESQUISAR USUARIOS -----\n");
    printf("1. Por Matricula\n");
    printf("2. Por Nome\n");
    printf("Escolha a opcao: ");
    if (scanf("%d", &opcao) != 1) {
        LimparBuffer();
        printf("Opcao invalida.\n");
        return;
    }
    LimparBuffer();

    if (opcao < 1 || opcao > 2) {
        printf("Opcao invalida.\n");
        return;
    }

    printf("Digite o termo de busca: ");
    scanf("%100[^\n]", termo_busca);
    LimparBuffer();

    // NOVO: Preparação para busca case-insensitive
    char termo_busca_lower[101];
    char campo_usuario_lower[101];
    
    // Faz a cópia do termo de busca e converte para minúsculas uma vez
    strncpy(termo_busca_lower, termo_busca, 100);
    termo_busca_lower[100] = '\0';
    strlwr_custom(termo_busca_lower);


    printf("\n--- RESULTADOS DA PESQUISA ---\n");
    printf("----------------------------------------------------------------------------------------\n");
    printf("| MATRICULA | NOME COMPLETO %-30s | CURSO %-15s | TELEFONE %-10s |\n", "", "", "");
    printf("----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_usuarios; i++) {
        int match = 0;
        switch (opcao) {
            case 1: // Por Matrícula
                if (atoi(termo_busca) == usuarios[i].matricula) match = 1;
                break;
            case 2: // Por Nome (case-insensitive)
                // Copia o nome completo do usuário, converte para minúsculas e busca
                strncpy(campo_usuario_lower, usuarios[i].nome_completo, 100);
                campo_usuario_lower[100] = '\0';
                strlwr_custom(campo_usuario_lower);
                if (strstr(campo_usuario_lower, termo_busca_lower) != NULL) match = 1;
                break;
        }

        if (match) {
            printf("| %-9d | %-45s | %-21s | %-16s |\n",
                   usuarios[i].matricula, usuarios[i].nome_completo, usuarios[i].curso, 
                   usuarios[i].telefone);
            encontrados++;
        }
    }
    printf("----------------------------------------------------------------------------------------\n");
    printf("Total de usuarios encontrados: %d\n", encontrados);
}


// Função que verifica todos os empréstimos ativos e os marca como ATRASADO, se necessário
void AtualizarStatusEmprestimos() {
    Data hoje = GerarDataAtual();
    int atrasados_agora = 0;

    for (int i = 0; i < num_emprestimos; i++) {
        // Apenas verifica empréstimos ATIVOS
        if (emprestimos[i].status == ATIVO) {
            // Se a data de hoje for maior que a data prevista de devolução
            if (CompararDatas(hoje, emprestimos[i].data_devolucao_prevista) > 0) {
                emprestimos[i].status = ATRASADO;
                atrasados_agora++;
            }
        }
    }
    if (atrasados_agora > 0) {
        printf("\n[AVISO: %d emprestimos foram marcados como ATRASADOS.]\n", atrasados_agora);
    }
}

// 7. Função para listar empréstimos ativos
void ListarEmprestimosAtivos() {
    int ativos = 0;
    
    // Verifica e atualiza status de atraso antes de listar
    AtualizarStatusEmprestimos();
    
    printf("\n----- LISTA DE EMPRESTIMOS ATIVOS/ATRASADOS -----\n");
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("| CODIGO | LIVRO COD | USUARIO MAT | EMPRESTIMO | DEVOLUCAO PREV. | STATUS  |\n");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_emprestimos; i++) {
        if (emprestimos[i].status == ATIVO || emprestimos[i].status == ATRASADO) {
            char status_str[10];
            if (emprestimos[i].status == ATIVO) strcpy(status_str, "ATIVO");
            else strcpy(status_str, "ATRASO");

            printf("| %-6d | %-9d | %-11d | %02d/%02d/%d | %02d/%02d/%d | %-7s |\n",
                   emprestimos[i].codigo, 
                   emprestimos[i].codigo_livro, 
                   emprestimos[i].matricula_usuario,
                   emprestimos[i].data_emprestimo.dia, emprestimos[i].data_emprestimo.mes, emprestimos[i].data_emprestimo.ano,
                   emprestimos[i].data_devolucao_prevista.dia, emprestimos[i].data_devolucao_prevista.mes, emprestimos[i].data_devolucao_prevista.ano,
                   status_str);
            ativos++;
        }
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
    if (ativos == 0) {
        printf("Nenhum emprestimo ativo ou atrasado encontrado.\n");
    } else {
        printf("Total de emprestimos ativos/atrasados: %d\n", ativos);
    }
}

// --- PARTE 5: FUNCIONALIDADES AVANÇADAS ---

// 8. Sistema de renovação de empréstimos
void RenovarEmprestimo() {
    int codigo_emprestimo, idx_emprestimo = -1;

    printf("\n----- RENOVAR EMPRESTIMO -----\n");

    if (num_emprestimos == 0) {
        printf("ERRO: Nao ha emprestimos registrados.\n");
        return;
    }

    printf("Codigo do Emprestimo a ser renovado: ");
    if (scanf("%d", &codigo_emprestimo) != 1) {
        printf("Entrada invalida.\n");
        LimparBuffer();
        return;
    }
    LimparBuffer();

    // Busca o empréstimo
    for (int i = 0; i < num_emprestimos; i++) {
        if (emprestimos[i].codigo == codigo_emprestimo) {
            idx_emprestimo = i;
            break;
        }
    }

    if (idx_emprestimo == -1) {
        printf("ERRO: Emprestimo #%d nao encontrado.\n", codigo_emprestimo);
        return;
    }

    // Verifica status do empréstimo
    if (emprestimos[idx_emprestimo].status == DEVOLVIDO) {
        printf("ERRO: Emprestimo #%d ja foi devolvido e nao pode ser renovado.\n", codigo_emprestimo);
        return;
    }
    
    // Verifica se já está atrasado (opcional: não renovar se estiver atrasado)
    AtualizarStatusEmprestimos(); // Garante status ATRASADO
    if (emprestimos[idx_emprestimo].status == ATRASADO) {
        printf("ERRO: Emprestimo #%d esta ATRASADO e deve ser devolvido antes de nova solicitacao.\n", codigo_emprestimo);
        return;
    }

    // Processa a Renovação
    Data nova_data_emprestimo = GerarDataAtual();
    Data nova_data_devolucao = CalcularDataDevolucao(nova_data_emprestimo);
    
    emprestimos[idx_emprestimo].data_emprestimo = nova_data_emprestimo;
    emprestimos[idx_emprestimo].data_devolucao_prevista = nova_data_devolucao;
    emprestimos[idx_emprestimo].status = ATIVO;

    printf("\n? Renovacao do Emprestimo #%d realizada com sucesso!\n", codigo_emprestimo);
    printf("    Nova data de devolucao prevista: %d/%d/%d\n", nova_data_devolucao.dia, 
           nova_data_devolucao.mes, nova_data_devolucao.ano);
    
    SalvarDados();
}

// 9. Relatório de livros mais emprestados
void RelatorioLivrosMaisEmprestados() {
    int contadores[MAX_LIVROS] = {0};
    int total_emprestimos = 0;

    printf("\n----- RELATORIO: LIVROS MAIS EMPRESTADOS -----\n");

    // 1. Contar quantas vezes cada livro foi emprestado
    for (int i = 0; i < num_emprestimos; i++) {
        int idx = BuscarLivroPorCodigo(emprestimos[i].codigo_livro);
        if (idx != -1) {
            contadores[idx]++;
            total_emprestimos++;
        }
    }
    
    if (total_emprestimos == 0) {
        printf("Nenhum emprestimo registrado.\n");
        return;
    }

    // 2. Cria um array de índices para ordenar
    int indices[MAX_LIVROS];
    for (int i = 0; i < num_livros; i++) indices[i] = i;

    // 3. Ordenação por contagem (Bubble Sort simples)
    for (int i = 0; i < num_livros - 1; i++) {
        for (int j = 0; j < num_livros - i - 1; j++) {
            if (contadores[indices[j]] < contadores[indices[j+1]]) {
                int temp = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = temp;
            }
        }
    }

    // 4. Exibe o relatório
    printf("--------------------------------------------------------------------------------\n");
    printf("| RANK | TITULO %-35s | AUTOR %-20s | EMPRESTIMOS |\n", "", "");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_livros && contadores[indices[i]] > 0; i++) {
        int idx = indices[i];
        printf("| %-4d | %-42s | %-26s | %-11d |\n",
               i + 1, livros[idx].titulo, livros[idx].autor, contadores[idx]);
    }
    printf("--------------------------------------------------------------------------------\n");
    printf("Total de registros de emprestimos: %d\n", total_emprestimos);
}


// 10. Relatório de usuários com empréstimos em atraso
void RelatorioUsuariosAtrasados() {
    // 1. Atualiza e identifica quem está atrasado
    AtualizarStatusEmprestimos();

    printf("\n----- RELATORIO: USUARIOS COM EMPRESTIMOS ATRASADOS -----\n");
    printf("------------------------------------------------------------------------------------------------\n");
    printf("| MATRICULA | NOME COMPLETO %-30s | LIVRO %-25s | DEVOLUCAO PREV. |\n", "", "", "");
    printf("------------------------------------------------------------------------------------------------\n");

    int encontrados = 0;
    for (int i = 0; i < num_emprestimos; i++) {
        if (emprestimos[i].status == ATRASADO) {
            int idx_usuario = BuscarUsuarioPorMatricula(emprestimos[i].matricula_usuario);
            int idx_livro = BuscarLivroPorCodigo(emprestimos[i].codigo_livro);

            if (idx_usuario != -1 && idx_livro != -1) {
                printf("| %-9d | %-45s | %-31s | %02d/%02d/%d |\n",
                       usuarios[idx_usuario].matricula,
                       usuarios[idx_usuario].nome_completo,
                       livros[idx_livro].titulo,
                       emprestimos[i].data_devolucao_prevista.dia,
                       emprestimos[i].data_devolucao_prevista.mes,
                       emprestimos[i].data_devolucao_prevista.ano);
                encontrados++;
            }
        }
    }
    printf("------------------------------------------------------------------------------------------------\n");
    if (encontrados == 0) {
        printf("Nenhum usuario com emprestimo em atraso no momento.\n");
    } else {
        printf("Total de emprestimos em atraso: %d\n", encontrados);
    }
}


// --- PARTE 2: SISTEMA DE MENUS E CONTROLE DE FLUXO ---

void MenuCadastros() {
    int opcao;
    do {
        printf("\n\n----- MENU CADASTROS -----\n");
        printf("1. Cadastrar Livro\n");
        printf("2. Cadastrar Usuario\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            LimparBuffer();
            opcao = -1; // Força repetição
        }
        LimparBuffer();

        switch (opcao) {
            case 1: 
					CadastrarLivro(); 
					break;
            case 2: 
					CadastrarUsuario(); 
					break;
            case 0: 
					printf("Voltando ao Menu Principal...\n"); 
					break;
            default: 
					printf("Opcao invalida. Tente novamente.\n"); 
					break;
        }
    } while (opcao != 0);
}

void MenuEmprestimos() {
    int opcao;
    do {
        printf("\n\n----- MENU EMPRESTIMOS -----\n");
        printf("1. Realizar Emprestimo\n");
        printf("2. Realizar Devolucao\n");
        printf("3. Renovar Emprestimo\n");
        printf("4. Listar Emprestimos Ativos/Atrasados\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            LimparBuffer();
            opcao = -1; // Força repetição
        }
        LimparBuffer();

        switch (opcao) {
            case 1: 
				RealizarEmprestimo(); 
				break;
            case 2: 
				RealizarDevolucao(); 
				break;
            case 3: 
				RenovarEmprestimo(); 
				break;
            case 4: 
				ListarEmprestimosAtivos(); 
				break;
            case 0: 
				printf("Voltando ao Menu Principal...\n"); 
				break;
            default: 
				printf("Opcao invalida. Tente novamente.\n"); 
				break;
        }
    } while (opcao != 0);
}

void MenuPesquisas() {
    int opcao;
    do {
        printf("\n\n----- MENU PESQUISAS -----\n");
        printf("1. Pesquisar Livros\n");
        printf("2. Pesquisar Usuarios\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            LimparBuffer();
            opcao = -1; // Força repetição
        }
        LimparBuffer();

        switch (opcao) {
            case 1: 
				PesquisarLivros(); 
				break;
            case 2: 
				PesquisarUsuarios(); 
				break;
            case 0: 
				printf("Voltando ao Menu Principal...\n"); 
				break;
            default: 
				printf("Opcao invalida. Tente novamente.\n"); 
				break;
        }
    } while (opcao != 0);
}

void MenuRelatorios() {
    int opcao;
    do {
        printf("\n\n----- MENU RELATORIOS -----\n");
        printf("1. Livros Mais Emprestados\n");
        printf("2. Usuarios com Emprestimos em Atraso\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            LimparBuffer();
            opcao = -1; // Força repetição
        }
        LimparBuffer();

        switch (opcao) {
            case 1: 
				RelatorioLivrosMaisEmprestados(); 
				break;
            case 2: 
				RelatorioUsuariosAtrasados(); 
				break;
            case 0: 
				printf("Voltando ao Menu Principal...\n"); 
				break;
            default: 
				printf("Opcao invalida. Tente novamente.\n"); 
				break;
        }
    } while (opcao != 0);
}


// Função principal do sistema
void MenuPrincipal() {
    int opcao;
    
    // Tenta carregar os dados salvos
    CarregarDados(); 
    
    do {
        printf("\n\n=============== SISTEMA DE BIBLIOTECA ===============\n");
        printf("1. Cadastros (Livros e Usuarios)\n");
        printf("2. Emprestimos e Devolucoes\n");
        printf("3. Pesquisas\n");
        printf("4. Relatorios Avancados\n");
        printf("0. Sair e Salvar\n");
        printf("=====================================================\n");
        printf("Escolha uma opcao: ");
        if (scanf("%d", &opcao) != 1) {
            LimparBuffer();
            opcao = -1; // Força repetição
        }
        LimparBuffer();

        switch (opcao) {
            case 1: 
				MenuCadastros(); 
				break;
            case 2: 
				MenuEmprestimos(); 
				break;
            case 3: 
				MenuPesquisas(); 
				break;
            case 4: 
				MenuRelatorios(); 
				break;
            case 0: 
                printf("\nSalvando dados finais e encerrando o sistema. Ate logo!\n");
                SalvarDados();
                break;
            default: 
                printf("Opcao invalida. Tente novamente.\n");
                break;
        }
    } while (opcao != 0);
}

// Função principal
int main() {
    // Inicializa a seed para funções de tempo (se necessário)
    srand(time(NULL));

    // Inicializa as estruturas e contadores (embora CarregarDados sobrescreva)
    InicializarEstruturas(); 
    
    // Inicia o loop principal do programa
    MenuPrincipal(); 
    
    return 0;
}

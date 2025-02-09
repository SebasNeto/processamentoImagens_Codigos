#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <numeric>
using namespace cv;

void filtro_sobel(Mat imagem, Mat& imagemFiltrada) {
    int altura = imagem.rows;
    int largura = imagem.cols;
    int canais = imagem.channels();

    float Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    float Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    imagemFiltrada = Mat::zeros(altura, largura, CV_32FC(canais));

    for (int canal = 0; canal < canais; canal++) {
        for (int i = 1; i < altura - 1; i++) {
            for (int j = 1; j < largura - 1; j++) {
                float gx = 0.0, gy = 0.0;

                for (int x = -1; x <= 1; x++) {
                    for (int y = -1; y <= 1; y++) {
                        int pixel = imagem.at<Vec3b>(i + x, j + y)[canal];
                        gx += Gx[x + 1][y + 1] * pixel;
                        gy += Gy[x + 1][y + 1] * pixel;
                    }
                }

                float gradiente = sqrt(gx * gx + gy * gy);
                imagemFiltrada.at<Vec3f>(i, j)[canal] = gradiente;
            }
        }
    }

    double minVal, maxVal;
    minMaxLoc(imagemFiltrada, &minVal, &maxVal);
    imagemFiltrada.convertTo(imagemFiltrada, CV_8UC(canais), 255.0 / maxVal);
}

void processarDiretorio(const char* input_dir, const char* output_dir, std::vector<double>& tempos_execucao) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(input_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".png") || strstr(ent->d_name, ".jpg") || strstr(ent->d_name, ".jpeg") || strstr(ent->d_name, ".bmp")) {
                char caminho_imagem[1024];
                snprintf(caminho_imagem, sizeof(caminho_imagem), "%s/%s", input_dir, ent->d_name);

                Mat imagem = imread(caminho_imagem, IMREAD_COLOR);
                if (!imagem.empty()) {
                    Mat imagemFiltrada;
                    clock_t start_time = clock();
                    filtro_sobel(imagem, imagemFiltrada);
                    clock_t end_time = clock();
                    double elapsed_time = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
                    tempos_execucao.push_back(elapsed_time);

                    char caminho_imagem_filtrada[1024];
                    snprintf(caminho_imagem_filtrada, sizeof(caminho_imagem_filtrada), "%s/%s", output_dir, ent->d_name);
                    imwrite(caminho_imagem_filtrada, imagemFiltrada);

                    printf("Tempo de processamento de %s: %.4f ms\n", ent->d_name, elapsed_time);
                } else {
                    printf("Erro ao carregar a imagem: %s\n", caminho_imagem);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Erro ao abrir diretório");
    }
}

void multiplasExecucoes(const char* input_dir, const char* output_dir, int execucoes) {
    std::vector<double> tempos_todas_execucoes;
    for (int execucao = 0; execucao < execucoes; execucao++) {
        printf("Iniciando execução %d\n", execucao + 1);
        std::vector<double> tempos_execucao;
        processarDiretorio(input_dir, output_dir, tempos_execucao);
        tempos_todas_execucoes.insert(tempos_todas_execucoes.end(), tempos_execucao.begin(), tempos_execucao.end());

        if (!tempos_execucao.empty()) {
            double media_execucao = std::accumulate(tempos_execucao.begin(), tempos_execucao.end(), 0.0) / tempos_execucao.size();
            printf("Média de tempo para a execução %d: %.4f ms\n", execucao + 1, media_execucao);
        } else {
            printf("Nenhuma imagem processada nesta execução.\n");
        }
    }

    if (!tempos_todas_execucoes.empty()) {
        double media_geral = std::accumulate(tempos_todas_execucoes.begin(), tempos_todas_execucoes.end(), 0.0) / tempos_todas_execucoes.size();
        printf("Média geral das médias de tempo após %d execuções: %.4f ms\n", execucoes, media_geral);
    } else {
        printf("Nenhuma imagem foi processada em nenhuma execução.\n");
    }
}

int main() {
    const char* input_dir = "/mnt/c/Users/Cliente/Downloads/base_dados/Imagens_Selecionadas";
    const char* output_dir = "/mnt/c/Users/Cliente/Downloads/base_dados/Saida_C_Sobel";
    
    multiplasExecucoes(input_dir, output_dir, 1);

    return 0;
}


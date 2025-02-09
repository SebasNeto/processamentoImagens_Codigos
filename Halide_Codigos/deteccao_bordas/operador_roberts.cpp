#include <vector>
#include <string>
#include <filesystem>
#include <numeric>
#include <chrono>

#include "Halide.h"
#include "halide_image_io.h"
#include <stdio.h>
#include <chrono>

using namespace Halide;
using namespace Halide::Tools;
using namespace std::chrono;
namespace fs = std::filesystem;

namespace fs = std::filesystem;

float processRobertsImage(const std::string& inputPath, const std::string& outputPath) {
    auto inicio_carregamento = std::chrono::high_resolution_clock::now();
    Buffer<uint8_t> input = load_image(inputPath);
    auto fim_carregamento = std::chrono::high_resolution_clock::now();

    if (!input.defined()) {
        printf("Erro ao carregar a imagem: %s\n", inputPath.c_str());
        return 0.0f;
    }

    Var x("x"), y("y"), c("c");

    // Definir as máscaras de Roberts para Gx e Gy como Func.
    Func roberts_x("roberts_x"), roberts_y("roberts_y");
    roberts_x(x, y) = select(
        x == 0 && y == 0, 1, x == 1 && y == 0, 0,
        x == 0 && y == 1, 0, x == 1 && y == 1, -1,
        0);
    roberts_y(x, y) = select(
        x == 0 && y == 0, 0, x == 1 && y == 0, 1,
        x == 0 && y == 1, -1, x == 1 && y == 1, 0,
        0);

    Func clamped = BoundaryConditions::repeat_edge(input);
    Func roberts_gx("roberts_gx"), roberts_gy("roberts_gy"), magnitude("magnitude");
    RDom r(0, 2, 0, 2); // Máscaras de Roberts são 2x2

    // Convolve com Gx
    roberts_gx(x, y, c) = sum(clamped(x + r.x, y + r.y, c) * roberts_x(r.x, r.y));

    // Convolve com Gy
    roberts_gy(x, y, c) = sum(clamped(x + r.x, y + r.y, c) * roberts_y(r.x, r.y));

    // Calcular a magnitude do gradiente (sqrt(gx^2 + gy^2)).
    magnitude(x, y, c) = cast<uint8_t>(sqrt(roberts_gx(x, y, c) * roberts_gx(x, y, c) +
        roberts_gy(x, y, c) * roberts_gy(x, y, c)));

    // Aplicar paralelismo e vetorização.
    roberts_gx.compute_root().parallel(y).vectorize(x, 16);
    roberts_gy.compute_root().parallel(y).vectorize(x, 16);
    magnitude.compute_root().parallel(y).vectorize(x, 16);

    auto inicio = std::chrono::high_resolution_clock::now();
    Buffer<uint8_t> output = magnitude.realize({ input.width(), input.height(), input.channels() });
    auto fim = std::chrono::high_resolution_clock::now();

    save_image(output, outputPath);

    auto duracao_processamento = std::chrono::duration_cast<std::chrono::milliseconds>(fim - inicio).count();
    std::string fileName = fs::path(inputPath).filename().string();
    printf("Duracao do processamento de %s: %ld ms\n", fileName.c_str(), duracao_processamento);
    return static_cast<float>(duracao_processamento);
}

int main(int argc, char** argv) {

    std::string inputDir = "/mnt/c/Users/Cliente/Downloads/base_dados/Imagens_Selecionadas";
    std::string outputDir = "/mnt/c/Users/Cliente/Downloads/base_dados/Saida_Halide_Roberts";

    std::vector<float> mediasGerais;

    // Pré-aquecimento
    for (int i = 0; i < 1; i++) {
        for (const auto& entry : fs::directory_iterator(inputDir)) {
            std::string inputPath = entry.path().string();
            std::string outputPath = outputDir + "\\" + entry.path().filename().string();
            processRobertsImage(inputPath, outputPath);
        }
    }

    for (int exec = 0; exec < 1; exec++) {
        std::vector<float> medias;

        for (const auto& entry : fs::directory_iterator(inputDir)) {
            std::string inputPath = entry.path().string();
            std::string outputPath = outputDir + "\\" + entry.path().filename().string();
            float duracao = processRobertsImage(inputPath, outputPath);
            medias.push_back(static_cast<float>(duracao));
            std::string fileName = fs::path(inputPath).filename().string();
            printf("Duracao da execucao para %s: %f ms\n", fileName.c_str(), duracao);
        }

        if (!medias.empty()) {
            medias.erase(medias.begin()); // Remove a primeira medição
        }

        float mediaExecucao = std::accumulate(medias.begin(), medias.end(), 0.0f) / medias.size();
        mediasGerais.push_back(mediaExecucao);
        printf("Media de tempo para a execucao %d: %f ms\n", exec + 1, mediaExecucao);
    }

    float mediaFinal = std::accumulate(mediasGerais.begin(), mediasGerais.end(), 0.0f) / mediasGerais.size();
    printf("Media geral das medias de tempo apos 1 execucoes: %f ms\n", mediaFinal);

    return 0;
}

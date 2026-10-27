# ⚙️ ParallelSha1Search

Сравнение моделей параллельного программирования на одной вычислительной задаче: поиск семисимвольной строки по SHA-1.

![C++](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white)
![Parallel](https://img.shields.io/badge/parallel-threads%20%7C%20MPI%20%7C%20CUDA-6A5ACD)

## Реализации

| Каталог | Модель | Платформа |
|---|---|---|
| `5_brute` | Последовательный перебор | macOS / Linux / Windows |
| `std_thread` | Потоки стандартной библиотеки | macOS / Linux / Windows |
| `pthread_create` | POSIX threads | macOS / Linux |
| `fork` | Отдельные процессы | macOS / Linux |
| `5_omp` | OpenMP | С компилятором и runtime OpenMP |
| `MPI_2` | Обмен MPI между процессами | С установленным MPI |
| `MPI_collective` | Коллективные операции MPI | С установленным MPI |
| `CreateThread` | WinAPI threads | Windows |
| `CreateProcess` | WinAPI processes | Windows |
| `cuda` | CUDA kernel | NVIDIA GPU + CUDA Toolkit |

В каждом каталоге сохранены код, реализация SHA-1, объяснение и примеры команд.

## Быстрый старт

Понадобятся компилятор C++17, CMake ≥ 3.18 и поддержка потоков для вашей платформы. Для smoke-проверок нужен Python ≥ 3.8. Базовая сборка не использует внешнюю библиотеку SHA-1: заголовки `sha1.hpp` находятся рядом с реализациями.

```bash
git clone https://github.com/lioiioiioil/ParallelSha1Search.git
cd ParallelSha1Search
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
printf '28a2f4f4403abf7b4e9bea6683a82ad529aca211\n' | ./build/std_thread
```

Ожидаемый ответ: `000000a`. Входной хеш должен быть записан строчными шестнадцатеричными символами. Базовая сборка включает только реализации для текущей платформы и не требует CUDA/MPI/OpenMP.

Дополнительные реализации включаются при конфигурации:

```bash
cmake -S . -B build-mpi -DENABLE_MPI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-mpi --parallel
printf '28a2f4f4403abf7b4e9bea6683a82ad529aca211\n' | mpiexec -n 9 ./build-mpi/mpi_collective
```

Аналогичные параметры: `-DENABLE_OPENMP=ON` и `-DENABLE_CUDA=ON`. Для Apple Clang OpenMP может потребовать настройки путей к libomp; команды прямой сборки сохранены в `implementations/5_omp/examples.txt`.

## Проверка

```bash
python3 scripts/smoke_test.py --build-dir build
# Или, если Python найден при конфигурации CMake:
ctest --test-dir build --output-on-failure
```

Скрипт проверяет базовые реализации для текущей платформы на трёх коротких примерах с известными ответами; каждый запуск ограничен по времени. Для многоконфигурационной сборки укажите `--build-dir build/Release`.

## Границы эксперимента

Алфавит: `0-9`, `a-z`, `A-Z`; длина строго 7, пространство поиска — 62⁷ комбинаций. Реализация `sha1.hpp` специализирована для семи байтов, это не универсальная библиотека SHA-1. Число потоков/процессов в ряде версий задано в коде. Проверка формата входа минимальна; полный перебор может занять очень много времени.

Проект показывает разные способы распределить одну и ту же работу. Готовых сопоставимых замеров производительности нет; результаты коротких проверок нельзя трактовать как сравнение ускорения.

## Проверка подготовки — 12 октября 2026

На macOS (Apple Silicon) успешно собраны и проверены `serial`, `std_thread`, `pthread_search` и `fork_search`: 3 известных входа на каждую реализацию, 12 успешных проверок. CUDA, MPI, OpenMP и Windows-варианты в этой подготовке не запускались.

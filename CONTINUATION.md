# CONTINUATION.md — ReXGlue SDK Android ARM64

Última atualização: 2026-11-11

## Atualização de continuidade — commit `82cc58cbda795d5f5ae566617a8be67f4b96a1b8`
- O Lint & Format Check após a primeira correção ainda falhou: o clang-format 20.1.8 continuou apontando as linhas 27 e 37 de `src/ui/surface_android.cpp`.
- A causa foi a quebra/alinhamento da assinatura de `GetSizeImpl` e a condição composta de validação do tamanho.
- Novo ajuste enviado: assinatura em uma linha e validações separadas, mantendo o mesmo comportamento.
- Commit mais recente: `82cc58cbda795d5f5ae566617a8be67f4b96a1b8`.
- Aguardar/verificar o novo Lint & Format Check disparado por esse commit; ainda não declarar lint aprovado.

## Objetivo
Registrar a adaptação do ReXGlue SDK para Android ARM64, com foco na superfície Vulkan baseada em `ANativeWindow`.

Repositório: https://github.com/joaovitoroutlok821-dot/rexglue-sdk
Branch: `android-arm64-fiber-experiment`

## Estado atual
A branch contém suporte inicial de plataforma Android no CMake e uma implementação de superfície nativa ligada à janela SDL3.

**A integração ainda não foi validada por compilação completa do SDK para Android ARM64.** Os commits abaixo registram alterações de código, não uma confirmação de build funcional.

## Alterações já realizadas
1. **CMake raiz — Android ARM64**
   - Commit: `94a4b9c425e081ef99e86041488648352d55fb24`
   - Adicionada seleção `ANDROID` antes do bloco UNIX genérico.
   - Identifica `android-arm64`, exige ARM64 e define `REX_PLATFORM_ANDROID=1` e `REX_PLATFORM_LINUX=1`.

2. **CMake UI**
   - Commit: `d251402c5729ce6a3eb2b5cac54814197df4a3de`
   - Seleciona ``surface_android.cpp` no Android.
   - Evita exigir X11/Wayland no Android.
   - Liga a biblioteca NDK `android`.

3. **Janela SDL3**
   - Commit: `ecd019099d45c22bad4f242e97dab29c8a9f3c9c`
   - Adiciona ramo Android em `window_sdl.cpp`.
   - Obtém `ANativeWindow*` pela propriedade `SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER` usando `SDL_GetPointerProperty`.

4. **Cabeçalho da superfície**
   - Commit: `2dacbc163ebde15490b4fc65e46b34d389d1ec87`
   - Cria `include/rex/ui/surface_android.h`.
   - Declara ``AndroidNativeWindowSurface`, derivada de `Surface`, com tipo `kTypeIndex_AndroidNativeWindow`.

5. **Implementação da superfície**
   - Commit inicial: `3594d6b844b41431534047227f6be28492167a44`
   - Cria `src/ui/surface_android.cpp`, adquire/libera referência de `ANativeWindow` e consulta dimensões com `SDL_GetWindowSizeInPixels`.

6. **Correção de formatação**
   - Commit: `c4a19d48c533bb443e4c4fd43b8228146625360d`
   - Ajusta as quebras de linha de `GetSizeImpl` para atender ao clang-format.

## Integração Vulkan identificada
Na leitura do código:
- `vulkan_presenter.cpp` já contém o caso `AndroidNativeWindowSurface` e chama `vkCreateAndroidSurfaceKHR`.
- `vulkan_instance.cpp` já habilita `VK_KHR_android_surface` quando `VK_USE_PLATFORM_ANDROID_KHR` está definido.
- `surface.h` já contém ``kTypeIndex_AndroidNativeWindow` e sua flag.
- SDL3 está fixado no submódulo em `147a8ee32dbf9ac02f3794964490687b6bbda1bc`; a propriedade Android foi localizada nessa versão.

Ainda falta confirmar por compilação que os macros, headers e dependências Vulkan Android estão disponíveis.

## CI — resultado observado
Workflow **Lint & Format Check #26**:
https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42930048030

- Run ID: `42930048030`
- Commit testado: `3594d6b844b41431534047227f6be28492167a44`
- Resultado: **failure**
- Job: `Format Check` (ID `130226196231`)
- O passo `Check C/C++ formatting` falhou em duas linhas de `src/ui/surface_android.cpp` (27 e 37).
- Correção enviada no commit `c4a19d48c533bb443e4c4fd43b8228146625360d`.
- O lint precisa ser executado novamente para confirmar a correção.

O teste Android ARM64 do Fiber valida somente `fiber_android_arm64.cpp` e seu assembly; não valida o SDK completo, UI ou Vulkan.

## Revisão do contrato Surface
Foi relido `include/rex/ui/surface.h`:
- O construtor base `Surface()` é protegido e default.
- `GetType() const` é virtual puro.
- `GetSizeImpl(uint32_t&, uint32_t&) const` é virtual puro protegido.
- `GetSize()` é público e valida dimensões zero.

A assinatura da implementação Android corresponde ao contrato observado.

## Pendências — retomar nesta ordem
1. Executar novamente o Lint & Format Check após o commit `c4a19d48c533bb443e4c4fd43b8228146625360d`.
2. Verificar os includes SDL3 em ``window_sdl.cpp` para `SDL_GetPointerProperty` e `SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER`.
3. Confirmar que `VK_USE_PLATFORM_ANDROID_KHR` é definido nos alvos Android; sem ele os tipos e funções Android Vulkan podem não ser expostos.
4. Auditar blocos CMake condicionados apenas por `UNIX`, detectando dependências Linux/X11/Wayland indevidas.
5. Revisar o ciclo de vida de `SDL_Window*` e `ANativeWindow*`; a superfície mantém referência ao native window, mas consulta o SDL window para dimensões.
6. Inspecionar `_build-platform.yaml` e workflows existentes para preparar compilação completa Android ARM64 com NDK e Vulkan headers/loader.
7. Rodar a compilação completa e corrigir os erros encontrados.
8. Só após build integral aprovado, integrar o SDK ao projeto Resident Evil Code Veronica e validar o APK no telefone.

## Critérios de conclusão
- ABI Android `arm64-v8a`.
- CMake configura sem dependências Linux/X11/Wayland indevidas.
- Compilação completa inclui UI e backend Vulkan Android.
- Superfície Vulkan Android compila e cria apresentação sobre `ANativeWindow`.
- Depois, APK integrado instala, apresenta imagem e mantém o ciclo de vida da janela sem crash.

## Ponto exato de retomada
O último problema conhecido de CI foi formatação em `surface_android.cpp), corrigida no commit `c4a19d48c533bb443e4c4fd43b8228146625360d`. Próximo passo: verificar o novo lint e então validar macros Vulkan e preparar o workflow de build Android completo.

## Atualização — correção da ABI do NDK (2026-11-11)
- Execução Android ARM64: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42933899469
- Lint & Format Check e Android ARM64 Fiber Backend Compile Check passaram no commit `91db57434320f7c06422868ba72477d5502a1e8f`.
- A configuração completa do SDK falhou em `CMakeLists.txt:101`: o preset mostrava `CMAKE_ANDROID_ARCH_ABI=arm64-v8a`, mas o toolchain NDK efetivamente selecionou `armeabi-v7a`.
- Correção enviada no commit `8d72c63ea0cb4e0d7b06b51ad8fe8127669524e0`: o preset agora define a variável documentada pelo NDK `ANDROID_ABI=arm64-v8a` em vez de tentar definir `CMAKE_ANDROID_ARCH_ABI` diretamente.
- Próximo passo: validar a nova execução. O build integral ainda não está aprovado; a etapa de compilação nem chegou a iniciar porque a configuração CMake foi interrompida pela ABI incorreta.

## Atualização — ABI ARM64 validada e habilitação de ASM (2026-11-11)
- Execução: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42939762979
- A alteração do preset para `ANDROID_ABI=arm64-v8a` resolveu a seleção incorreta de ABI. O CMake avançou e exibiu `Configuring done`.
- Lint & Format Check passou: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42939763030
- A geração falhou depois da configuração porque o projeto não havia habilitado a linguagem ASM: variável interna ausente `CMAKE_ASM_COMPILE_OBJECT`.
- Correção enviada no commit `af580822002d85fde899c9c683a66aa41ba8194a`: ASM foi incluída nas linguagens do projeto para builds normais e Apple.
- Próximo passo: verificar a nova execução e corrigir o próximo erro real, se houver. A compilação integral ainda não foi concluída.


## Atualização — compatibilidade de conversão float/double no Android (2026-11-11)
- Execução Android ARM64: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42942946133
- O CMake configurou corretamente para Android ARM64 e o Ninja iniciou a compilação (mais de 420 de 790 tarefas foram alcançadas).
- O novo bloqueio ocorreu em `include/rex/string/numeric.h`: o libc++ fornecido pelo NDK 27.2.12479018 não disponibilizou as sobrecargas de ponto flutuante de `std::from_chars`, causando erros para `float` e `double`.
- A implementação já possuía um parser portátil baseado em locale C para macOS. O commit `13b95a3e3d6f4cd201f79e4dec51fb5208a96e16` estendeu esse caminho também ao Android, usando `strtof_l`/`strtod_l` e mantendo a validação de entrada e erros.
- O parser hexadecimal e as conversões integrais continuam usando `std::from_chars`.
- Próximo passo: aguardar lint e build Android ARM64 no commit acima; corrigir somente o próximo erro confirmado.
- Status: build completo ainda não aprovado; artefato SDK ainda não foi gerado.


## Atualização — fallback de conversão de relógios C++23 no Android (2026-11-11)
- Build anterior: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42943816809
- A correção do parser numérico foi compilada sem erros nos pontos que chegaram a ser construídos; o build avançou até cerca de 423/790 tarefas.
- O novo erro apareceu em `include/rex/chrono/chrono.h`: a libc++ do Android NDK não declara `std::chrono::clock_time_conversion`, embora o código do projeto precise especializar esse template.
- O arquivo já continha um fallback equivalente para Apple libc++. O commit `25ad94540c9c556a3b9fe63ab6c5c3ebb1764a20` estendeu o mesmo fallback para Android (`__ANDROID__`), mantendo as conversões WinSystemClock/XSystemClock existentes.
- Próximo passo: validar lint e build Android ARM64 neste commit; depois tratar o próximo erro real.
- Status: build integral ainda não concluído e nenhum artefato SDK foi publicado.


## Atualização — compatibilidade de thread no Android NDK (2026-11-11)
- Build analisado: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42945128946
- CMake configurou corretamente e o Ninja avançou até aproximadamente 487/790 tarefas.
- O bloqueio confirmado foi em `src/core/timer_queue.cpp`: o libc++ do NDK 27.2.12479018 não fornece `std::jthread` nem `std::stop_token` nessa configuração.
- Correção enviada no commit `6886817aa63447809fa5c3e9c301fe34476a4b29`: substituído `std::jthread` por `std::thread`, com sinalizador atômico de parada, despertar da fila por item sentinela e `join()` explícito no destrutor. O tipo de ID da thread foi ajustado para `std::thread::id`.
- Próximo passo: verificar lint e build Android ARM64 disparados pelo commit e corrigir apenas o próximo erro confirmado.
- Status: compilação integral ainda não concluída; nenhum artefato SDK foi gerado.


## Atualização — APIs de arquivos grandes no Android (2026-11-11)
- Build analisado: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42946044376
- A adaptação de `TimerQueue` compilou; a execução avançou até aproximadamente 497/790 tarefas e compilou também o backend fiber Android ARM64 em C++ e assembly.
- O novo bloqueio confirmado foi em `src/core/filesystem_posix.cpp`: o Android NDK não declara `fseeko64`/ `ftello64`; disponibiliza as variantes POSIX `fseeko`/`ftello`. Em Android ARM64, `off_t` já é 64-bit.
- Correção enviada no commit `d4c68c0a74cb1971b52ef1cbc0a3f2758c2c3341`: Android foi incluído junto com Apple no ramo que usa `off_t`, `fseeko`, `ftello` e `ftruncate`; Linux mantém as variantes explícitas `*64` para compatibilidade com distribuições antigas de 32 bits.
- Próximo passo: verificar a nova compilação e o lint, tratando o próximo erro confirmado.
- Status: SDK ainda não foi compilado integralmente e não há artefato publicado.


## 2026-11-11 — Android shared-memory API detection

- The Android ARM64 build reached task 504/790 and failed because `src/core/memory_posix.cpp` referenced `rex::GetAndroidApiLevel()`, which is not declared or implemented in this SDK branch.
- Removed that undeclared dependency. `AndroidInitialize()` now opens `libandroid.so` and resolves `ASharedMemory_create` directly with `dlsym`.
- This detects API 26+ by symbol availability and preserves the existing `/dev/ashmem` fallback when the symbol is unavailable, without requiring a separate Android API-level helper.
- Source commit: `7fd7b72fb0a2d6ce27924cbd031aeb77f48c5a73`.
- Validation: pending the next Android ARM64 build and lint workflow runs.


## 2026-11-11 — Detecção de pthread_getname_np no Android

- A compilação Android ARM64 avançou até a tarefa 506/790 e falhou em `src/core/threading_posix.cpp` porque o include `rex/main_android.h` não existe e o código dependia de `rex::GetAndroidApiLevel()`.
- Removido o include inexistente e substituída a consulta de nível de API pela resolução dinâmica de `pthread_getname_np` via `dlopen("libc.so")` e `dlsym`, preservando o comportamento opcional em versões sem o símbolo.
- Alteração registrada no commit `776ce4d3a579864a168517732d988c009137a326`.
- Próximo passo: validar lint e executar novamente a compilação Android ARM64; ainda não há artefato SDK confirmado.

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


## 2026-11-11 — Compatibilidade de mutexes e sinais POSIX no Android

- A compilação passou pela configuração e chegou à tarefa 507/790. O erro seguinte em `threading_posix.cpp` foi causado por macros Linux ativadas também no Android: o NDK não declara `PTHREAD_MUTEX_ROBUST` nem `pthread_mutex_consistent`; também não declara a chamada `sigqueue` usada nesse ramo.
- Restringi os trechos de mutex robusto ao Linux que não é Android. No Android, o sinal de callback agora usa `pthread_kill`, suficiente porque o handler trata esse sinal apenas como aviso para acordar e drenar callbacks na espera alertável; o payload não era consumido pelo handler.
- Alteração enviada no commit `96fab70ab9b0ae2691bac3224d40785cc35df09d`.
- Lint e nova compilação ainda precisam ser confirmados. Nenhum artefato SDK foi gerado até aqui.


## 2026-11-11 — Retorno seguro de fibras Android ARM64

- A revisão do backend confirmou que o TLS é atualizado antes de cada troca e que o assembly preserva x19–x30, SP, d8–d15, FPCR e FPSR.
- Foi identificado um risco real no término da função de entrada: o trampoline terminava o processo com `std::terminate`, sem retornar cooperativamente ao scheduler/à fibra que ativou a fibra.
- O backend Android agora registra a fibra ativadora na primeira troca. Quando a função de entrada retorna, marca a fibra como concluída e troca de volta para a fibra ativadora; tentativas posteriores de retomar uma fibra concluída são ignoradas.
- Adicionado teste unitário que valida retorno ao ativador, TLS correto e bloqueio de retomada após conclusão.
- Esta alteração trata o ciclo de vida de fibras, não representa ainda um Save State completo do jogo. O snapshot de registradores/pilha para persistência em disco continua sendo uma etapa separada.
- Validação pendente: lint, teste unitário e compilação do backend Android ARM64.


## 2026-11-11 — Android SAF file-descriptor bridge

- O build completo chegou à ligação de `librexruntime.so` (689/790) e falhou por símbolo não definido: `rex::filesystem::OpenAndroidContentFileDescriptor`.
- A declaração já existia em `include/rex/filesystem.h`, e `MappedMemory::OpenForAndroidContentUri` já dependia dela, mas não havia implementação Android no alvo.
- Adicionado um ponto de integração explícito para o app Android registrar um callback SAF (`SetAndroidContentUriOpener`). O callback recebe a URI e o modo (`r` ou `rw`) e deve devolver um descritor recém-aberto cuja propriedade é transferida ao SDK.
- A implementação retorna erro seguro quando o app ainda não registrou o callback; não simula acesso a arquivos.
- Próxima integração necessária no app Resident Evil: registrar o callback JNI usando ContentResolver e ParcelFileDescriptor, mantendo a permissão persistida da URI. Depois testar abertura, mmap, flush e fechamento.
- Validação pendente: build Android ARM64 e lint. A ponte nativa não substitui ainda o registro JNI no aplicativo.


## 2026-11-11 — Correção do parser float/double no CI Linux

- Guest Frame Prototype Unit Tests #4: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43077240114
- O job `unit-tests` falhou durante `Build unit tests`, antes de executar os testes. O log confirma compilador `/usr/bin/clang++-18`, libc++ e macro `REX_PLATFORM_LINUX=1`.
- A causa não era uma condição Android: o fallback `portable_float_from_chars` já existia para macOS/Android, mas não era ativado no Linux do runner. Assim, as conversões de `float` e `double` em `numeric.h` tentavam usar sobrecargas de `std::from_chars` ausentes nessa libc++.
- Correção enviada: incluir `REX_PLATFORM_LINUX` nas quatro condições de compilação que habilitam `locale.h`, o parser C-locale e seu uso em `float`, `double` e `vec128_t`. O parser mantém locale C explícito e a validação existente; os caminhos integrais/hexadecimais continuam inalterados.
- Commit da correção: `6940da25ff175c183e8fc72136828bf5c844bded`.
- Próximo passo: verificar os novos workflows de lint e Guest Frame Unit Tests disparados pelo commit. Os testes de frame ainda não estão confirmados como aprovados.


- Ajuste adicional de CI: o filtro `paths` do workflow de testes não incluía `include/rex/string/numeric.h`; por isso, a alteração do parser não dispararia automaticamente o teste específico. O cabeçalho foi incluído no filtro.
- Commit do ajuste de workflow: `80489070dce5a9b16ee1f1f1e1a125ccb87c195d`.
- A execução do Guest Frame Prototype Unit Tests deve ser conferida no novo run; não considerar a correção validada até o build e o CTest terminarem com sucesso.


## 2026-11-11 — Fallback de clock_cast para libc++ no Linux (Guest Frame CI)

- Guest Frame Prototype Unit Tests #5: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43110121690
- A correção anterior em `include/rex/string/numeric.h` eliminou os erros de `std::from_chars`; a compilação avançou até aproximadamente 533/929 tarefas.
- O novo bloqueio ocorreu em `include/rex/chrono/chrono.h`: o Clang 18 com libc++ no Ubuntu 24.04 não declara `std::chrono::clock_time_conversion`/ `clock_cast`, mas o cabeçalho especializa esse template para converter `WinSystemClock` e `XSystemClock`.
- O próprio arquivo já fornece um fallback equivalente para Apple e Android. A condição foi estendida também a Linux (`__linux__`), reutilizando o mesmo fallback sem alterar as conversões específicas dos relógios.
- O filtro `paths` do workflow Guest Frame também foi atualizado para incluir `include/rex/chrono/chrono.h`, garantindo que essa correção dispare a validação.
- Próximo passo: verificar o novo workflow de testes e os checks de lint. O CTest ainda não foi executado e o protótipo de Save State continua não validado.


## 2026-11-11 — Compatibilidade Catch2/libc++ para __int128

- Guest Frame Prototype Unit Tests #6: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43112575765
- As correções de `numeric.h` e `chrono.h` permitiram que o build chegasse a 922/929 tarefas. O bloqueio seguinte foi no Catch2, ao formatar `std::chrono::duration` cujo tipo de representação é `__int128`; libc++ não fornece um operador `std::ostream << __int128`, causando ambiguidade.
- Adicionado `tests/unit/catch2_compat.h` com operadores de saída decimal para `__int128` e `unsigned __int128`, incluindo tratamento seguro do menor valor signed.
- O cabeçalho é forçado apenas no alvo `unit_tests` por `tests/unit/CMakeLists.txt`; não altera o runtime/SDK nem o APK. O workflow também monitora o novo cabeçalho.
- Próximo passo: aguardar Lint e Guest Frame Unit Tests #7. A compilação anterior ainda não chegou ao CTest; Save State não está validado.


## 2026-11-11 — Guest Frame Prototype Unit Tests #9 aprovado

- Execução: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43121652025
- Commit testado: `7c70830687057b00c3fd684529573af797e3187a`.
- Resultado do job `unit-tests`: **success**.
- `Configure test build`: passou.
- `Build unit tests`: passou.
- `Run guest frame tests`: passou.
- O Lint & Format Check #86 também passou no mesmo commit: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43121652188
- A correção final de formatação em `tests/unit/catch2_compat.h` foi registrada no commit `7c70830687057b00c3fd684529573af797e3187a`.
- Interpretação: o protótipo atual de pilha de chamadas convidadas passou pelos testes unitários disponíveis. Isso **não** comprova Save State completo, persistência em disco, restauração após reiniciar o processo nem integração com o jogo Resident Evil.
- Próxima etapa: revisar a implementação e os limites do snapshot de guest call stack, identificar o estado adicional do runtime que precisa ser serializado e só então planejar integração no app RECVX já funcional. Não reconstruir o jogo do zero.


## 2026-11-11 — Revisão técnica após os testes aprovados

- Revisados `include/rex/ppc/guest_call_stack.h`, `tests/unit/ppc/guest_call_stack_test.cpp` e `src/codegen/function_graph.cpp`.
- Confirmação importante: o próprio cabeçalho descreve `GuestCallStack` como experimental e **não conectado ao PPC codegen nem à execução do runtime**.
- Os testes exercitam apenas metadados de frames: iniciar, chamada, retorno, atualizar resume PC e copiar/restaurar um vetor em memória. Não executam instruções PPC nem comprovam captura/restauração de registradores, memória, scheduler, threads ou estado gráfico/áudio.
- Portanto, o resultado verde do workflow valida o modelo isolado, não um Save State utilizável.
- Próximo passo técnico: mapear o caminho real de execução das funções recompiladas (emissão de chamadas/retornos, macros de função, dispatcher e estado PPC por thread) para decidir onde instrumentar continuations sem quebrar o jogo existente. Depois ampliar os testes para execução/resume real antes de qualquer integração no RECVX.


## 2026-11-11 — Mapeamento do caminho de execução PPC e threads

- Revisados `src/system/function_dispatcher.cpp`, `src/system/xthread.cpp`, `include/rex/system/xthread.h`, `include/rex/system/thread_state.h` e a estrutura `PPCContext` em `include/rex/ppc/context.h`.
- `FunctionDispatcher::Execute(ThreadState*, address)` resolve o endereço convidado para um `PPCFunc*`, ajusta `r1`/ `lr`, chama a função recompilada diretamente no host (`fn(*ctx, membase)`) e restaura `lr`/ `r1` ao retornar.
- `XThread::Execute()` também chama diretamente o ponteiro `PPCFunc*` de entrada. Assim, chamadas internas emitidas como chamadas C++ criam frames na pilha nativa do host; não existe hoje um dispatcher cooperativo por instrução que possa simplesmente gravar um PC e retomar.
- `ThreadState` mantém um `PPCContext` por thread. O contexto inclui GPRs, LR, CTR, XER, MSR, CRs, FPSCR, FPRs, vetores e `last_indirect_target`. Esses registradores são parte do estado necessário, mas não bastam para restaurar a pilha C++ do host.
- `XThread::Save()` retorna falso para thread convidada em execução; há TODO explícito sobre serialização compatível com ReXGlue. O caminho `Restore()` cria thread suspensa, mas o código da thread restaurada ainda registra “Thread restore not implemented”.
- `XThread::Suspend()` usa suspensão da thread host; isso não converte uma pilha C++ ativa em uma continuação serializável e não deve ser confundido com checkpoint seguro.
- Conclusão de arquitetura: o protótipo atual de `GuestCallStack` não pode ser ligado apenas ao `FunctionDispatcher` como se isso preservasse os frames C++ já ativos. Uma solução verdadeira exige transformar limites de função/chamadas convidadas em pontos de continuação cooperativos (por exemplo, dispatcher/trampolins com estado explícito e retorno controlado), ou outra estratégia que não dependa de restaurar a pilha nativa entre processos.
- Antes de implementar, mapear macros/helpers de emissão de chamadas e todos os caminhos de execução (chamada direta, indireta, tail call, imports, traps/interrupts, fibras e APCs), definindo quais operações podem constituir pontos seguros. O snapshot também precisará incluir memória convidada, PPCContext, estado de threads, TLS/stack, objetos/kernel, temporizadores e estado de áudio/vídeo/input conforme aplicável.
- Nenhuma mudança foi feita no app Resident Evil; preservá-lo como está até existir uma camada de checkpoint validada.


## 2026-11-11 — Mapeamento das instruções de fluxo e chamadas emitidas pelo codegen

- Revisados `src/codegen/function_graph.cpp`, `src/codegen/builders/control_flow.cpp`, `src/codegen/builders/context.cpp`, `include/rex/codegen/function_node.h`, `include/rex/ppc/func.h` e `include/rex/ppc/function.h`.
- `FunctionNode::emitCpp()` gera uma função C++ por função PPC, inicia com `REX_FUNC_PROLOGUE()` e emite instruções como código C++ comum. Os labels internos viram labels C++ e `goto loc_...`; variáveis de registradores podem ser localizadas em variáveis automáticas C++.
- Fluxo mapeado:
  - `bl`: grava LR e, quando resolvido, chama diretamente a função C++ do destino ou wrapper de import.
  - `b`: salto interno via `goto` ou chamada de cauda seguida de `return`.
  - `blr`: `return` da função C++.
  - `bctrl`/`blrl`: chamada indireta por `REX_CALL_INDIRECT_FUNC`.
  - `bctr`: switch de jump table para labels/funções ou chamada indireta de cauda.
  - chamadas condicionais e helpers de setjmp/longjmp/SEH/APC são caminhos adicionais que precisam ser considerados.
- `BuilderContext::emit_function_call()` emite chamadas diretas C++ para funções e imports; a chamada direta não passa pelo `FunctionDispatcher::Execute()`. O dispatcher é usado em pontos de entrada, callbacks e execução externa, não em todas as chamadas PPC internas.
- Os wrappers host/PPC em `include/rex/ppc/function.h` também executam funções nativas C++ e fazem tradução de argumentos/retornos, portanto imports e callbacks precisam participar do desenho de checkpoint.
- Implicação: não é seguro simplesmente colocar um push/pop de frame em `FunctionDispatcher`; isso não observa todas as chamadas internas. E guardar só `PPCContext`/PC não preserva variáveis locais nem a pilha C++ que contém os frames ativos.
- Direção arquitetural a avaliar: transformar funções recompiladas em continuations explícitas/state machines, com PC convidado de retomada e armazenamento explícito dos registradores locais/live values por frame. Chamadas diretas, indiretas, tail calls, imports e retornos precisam usar protocolo consistente. Alternativamente, estudar checkpoint cooperativo em pontos onde a execução inteira tenha desenrolado até um scheduler seguro, sem prometer restauração de pilha nativa.
- Não implementar ainda uma alteração invasiva no gerador. Primeiro elaborar uma proposta incremental e testes de equivalência de execução para um pequeno conjunto de funções PPC, incluindo chamada direta, chamada indireta, retorno, loop/branch interno e import. Só depois ampliar para threads/fibras e snapshot de memória/kernel.
- Estado atual continua sendo protótipo experimental; nenhum Save State funcional foi integrado ao jogo RECVX.


## 2026-11-11 — Fibras Android ARM64 e serialização de threads

- Revisados `include/rex/thread/fiber.h`, `src/core/fiber_android_arm64.cpp`, `src/core/fiber_android_arm64.S`, `tests/unit/core/fiber_test.cpp` e os caminhos `XThread::Save/Restore`.
- Android ARM64 já possui troca cooperativa de contexto de fibra: salva/restaura x19-x30, SP, d8-d15, FPCR e FPSR. A fibra mantém sua pilha em um `std::vector<uint8_t>` e os testes provam alternância/suspensão/retomada dentro do mesmo processo.
- Isso não prova serialização persistente. O contexto contém SP e registradores host; a pilha contém frames C++ ativos, endereços de retorno e potencialmente ponteiros para objetos/locais. Ao restaurar em outro processo, a pilha alocada pode estar em outro endereço e copiar bytes não relocaliza de forma geral os ponteiros internos. Não tratar snapshot de `Fiber` como Save State portátil.
- `XThread::Save()` retorna `false` imediatamente quando `running_` é verdadeiro, com aviso de serialização não implementada. O código subsequente que tenta salvar o contexto sob `if (running_)` é atualmente inalcançável por esse retorno.
- `XThread::Restore()` restaura alguns metadados e PPCContext, cria uma thread suspensa, mas o corpo de execução termina com `REXSYS_ERROR("Thread restore not implemented")`; além disso, o estado serializado atual não fornece um PC válido de continuação.
- A camada Fiber é útil para criar pontos de pausa cooperativa e testes intra-processo, mas não resolve persistência após encerramento/reinício. Um protótipo de persistência deve usar continuations explícitas e dados serializáveis, ou demonstrar uma estratégia de relocalização completa antes de considerar pilha host.
- Critério mínimo para um Save State verdadeiro: pausar em ponto seguro, salvar estado guest + memória + scheduler/threads e objetos relevantes; fechar completamente o processo; iniciar novamente, reconstruir o runtime e retomar do PC/estado serializado; comprovar continuidade determinística sem depender da pilha C++ antiga.


## 2026-11-11 — Proposta incremental para continuations persistentes no codegen

### Evidência adicional do gerador
- `FunctionNode::emitCpp()` em `src/codegen/function_graph.cpp` emite uma função C++ comum, chama `REX_FUNC_PROLOGUE()`, gera labels C++ (`loc_...`) e declara registradores localizados como variáveis automáticas no escopo da função.
- Os caminhos de chamadas são emitidos por `BuilderContext::emit_function_call()` em `src/codegen/builders/context.cpp`; funções resolvidas e imports podem ser invocados diretamente como funções C++.
- Logo, uma retomada persistente não pode depender de labels C++ nem de variáveis automáticas que desaparecem quando a função retorna/processo termina.

### Arquitetura candidata (ainda não implementada)
1. Definir um frame de execução serializável, identificado por versão e endereço guest da função, contendo PC guest de retomada, PPCContext/registradores vivos, estado de retorno e slots para valores temporários/localizados.
2. Fazer o codegen produzir uma variante experimental de função resumível: blocos básicos identificados por PC, transições explícitas e armazenamento de todos os valores vivos no frame antes de qualquer ponto de yield/chamada que possa suspender.
3. Usar um dispatcher de continuations para executar um passo/quantum e empilhar frames guest explícitos nas chamadas diretas e indiretas; retorno remove o frame e propaga os valores de retorno.
4. Imports e callbacks precisam declarar se são checkpoint-safe. Operações com efeitos externos não podem ser interrompidas no meio; só podem ser capturadas antes/depois de um limite seguro.
5. Manter a emissão atual como caminho padrão. O modo resumível deve ser opt-in e limitado inicialmente a funções de teste sem SEH, setjmp/longjmp, APC, callbacks reentrantes ou instruções não suportadas.
6. Persistir a pilha de frames guest e o estado de scheduler junto da memória guest e dos objetos necessários. No boot de restauração, reconstruir ponteiros host e recursos, nunca desserializar endereços de função/ponteiros host crus.
7. Só depois de testes determinísticos em processo separado, ampliar cobertura para SEH, imports reais, threads, APC/fibras e estado de kernel/áudio/vídeo.

### Primeiro protótipo de validação sugerido
- Criar uma função PPC sintética com sequência aritmética, branch/loop interno e retorno.
- Executá-la no caminho atual e registrar resultado/estado final de referência.
- Executar a variante resumível em pequenos quanta, serializando o frame em cada pausa; destruir o estado em memória, reconstruir a partir dos bytes serializados e continuar.
- Comparar registradores finais, memória guest modificada, PC/retorno e número de iterações com a execução de referência.
- Adicionar casos separados para chamada direta, indireta e import stub; rejeitar explicitamente instruções/caminhos não suportados.
- O teste decisivo de persistência deve ocorrer em outro processo (ou execução isolada do teste), não apenas copiar uma struct e continuar no mesmo processo.

### Riscos e limites conhecidos
- Transformar o gerador inteiro em state machine é uma alteração ampla; a primeira prova deve ser um gerador/test fixture mínimo isolado, não recompilar todo o jogo.
- O conjunto de valores vivos deve ser calculado/validado por bloco; salvar apenas PPCContext não basta se o codegen mantém valores em locais.
- SEH, exceções C++, setjmp/longjmp, mid-asm hooks, imports com efeitos colaterais, callbacks/APCs e sincronização concorrente exigem protocolos próprios.
- Fibers Android ARM64 permanecem úteis para pausa cooperativa intra-processo, mas não são o formato de persistência.
- Nenhuma alteração foi feita no caminho de execução normal nem no app Resident Evil. Nenhuma funcionalidade Save State foi declarada pronta.


### 2026-11-11 — Restrições do primeiro protótipo de codegen
- `RecompilerConfig` permite localizar registradores em variáveis locais: `ctrAsLocalVariable`, `xerAsLocalVariable`, `reservedRegisterAsLocalVariable`, `crRegistersAsLocalVariables`, `nonArgumentRegistersAsLocalVariables` e `nonVolatileRegistersAsLocalVariables`.
- Para uma variante resumível inicial, todas essas opções devem ser forçadas a `false`, fazendo GPR/FPR/VMX/CR/CTR/XER/reserved permanecerem em `PPCContext`.
- Isso ainda não resolve os temporários internos do gerador (`temp`, `vTemp`, `env`, `ea`) nem valores de expressão. A transformação deve garantir que nenhum desses valores permaneça vivo entre yields; caso contrário, precisam de slots explícitos no frame.
- O primeiro escopo deve rejeitar funções com SEH, mid-asm hooks, setjmp/longjmp, jump tables não suportadas, chamadas/imports e instruções sem lowering resumível. Inicialmente testar apenas aritmética + branch condicional/incondicional dentro de uma função, com estado explícito no frame.
- Os campos do frame devem ser serializados por formato versionado e campos definidos, não por dump bruto de `PPCContext`: há padding/alinhamento e campos como FPSCR carregam estado de controle do host. O formato persistente precisa representar os bits guest e reconstruir o estado host ao restaurar.
- Decisão: o primeiro teste de equivalência deve comparar uma execução de referência e uma execução resumível de uma função sintética sem chamadas, interrompida entre blocos básicos. Só após isso adicionar chamadas e imports com protocolo explícito.


## 2026-11-11 — Auditoria detalhada do ponto de emissão do codegen resumível

- Revisados novamente `FunctionNode::emitCpp()` em `src/codegen/function_graph.cpp`, `BuilderContext` em `src/codegen/builders/builder_context.h` e `src/codegen/builders/context.cpp`, os builders de fluxo em `src/codegen/builders/control_flow.cpp`, além de `FunctionNode` e `FunctionGraph`.
- A emissão atual percorre cada intervalo `Block` linearmente, decodifica cada instrução e chama `DispatchInstruction()`. Os destinos internos são emitidos como labels C++ `loc_XXXXXXXX:` e saltos `goto`; não existe ainda uma representação de blocos básicos executáveis como estados independentes.
- O corpo é montado em uma string e, ao final, o gerador declara variáveis automáticas detectadas em `RecompilerLocalVariables` (`rN`, `fN`, `vN`, `crN`, `ctr`, `xer`, `temp`, `vTemp`, `env`, `ea`) antes de anexar o corpo. Essas variáveis sobrevivem entre instruções somente pela pilha C++ da chamada atual.
- Os builders de branch interno emitem `goto` para labels; `blr` emite `return`; `bl` para função/import chama diretamente a função C++ gerada ou wrapper. `bctrl`/`blrl` usam dispatch indireto; jump tables geram `switch` e podem saltar a labels ou chamar funções. Logo, uma transformação resumível precisa substituir o fluxo inteiro de forma coerente, e não apenas inserir um `yield` na saída do builder.
- O primeiro protótipo não deve reutilizar o corpo C++ atual com `goto`: deve criar uma segunda via de emissão opt-in que converta cada bloco básico suportado em um estado explícito (PC guest) e cada transição em atualização do PC/retorno ao dispatcher. Antes de emitir essa variante, precisa validar que todos os valores usados depois de uma transição estão no frame ou em estado guest persistente.
- Não foi alterado o codegen nem o runtime nesta etapa. Nenhuma alteração foi feita no caminho padrão e nenhuma integração foi feita no aplicativo RECVX.
- Próxima etapa: definir um formato mínimo de frame versionado e um pequeno fixture de função PPC sintética (aritmética + branch/loop interno, sem chamadas, imports, SEH, hooks ou jump tables); provar equivalência entre a execução C++ normal e a máquina de estados antes de adicionar serialização/deserialização e reconstrução a partir de bytes.


## 2026-11-11 — Primeiro formato explícito de frame resumível (protótipo isolado)

- Adicionado `include/rex/ppc/resumable_frame.h` com `rex::ppc::ResumableFrame`: formato binário versionado (`RXRF`, versão 1), endereços guest da função/PC de retomada, GPRs, LR, CTR, bits guest de XER, campos CR e slots temporários explícitos.
- A serialização escreve cada campo individualmente em little-endian; não faz dump bruto de `PPCContext`, não inclui padding nem ponteiros host. A leitura rejeita magic/versão incorretos, endereços nulos, campos inválidos, slots acima do limite e bytes extras/truncados.
- O escopo atual é deliberadamente parcial: ainda não inclui FPR, VMX, FPSCR, memória guest, scheduler, threads, estado de kernel, gráficos ou áudio. Portanto, esse frame não representa ainda um Save State completo.
- Adicionado `tests/unit/ppc/resumable_frame_test.cpp` com round-trip de estado, rejeição de payloads inválidos e uma fixture manual de máquina de estados com loop/branch. A fixture interrompe a execução, serializa, destrói o objeto original, reconstrói a partir dos bytes e compara o resultado com uma implementação de referência.
- Importante: a fixture é escrita manualmente em C++ e **não é código PPC gerado pelo FunctionNode**. Ela valida o contrato de persistência do frame e a semântica básica de retomada, não prova ainda a transformação do codegen.
- Registrado o novo teste em `tests/unit/CMakeLists.txt` e no workflow `.github/workflows/guest-frame-prototype-tests.yml`.
- Nenhuma alteração foi feita em `FunctionNode::emitCpp()`, nos builders normais, no dispatcher ou no runtime. O caminho padrão e o APK funcional do RECVX permanecem fora desta experiência.
- Próximo passo após CI verde: ligar o modelo a um emissor experimental de blocos básicos, inicialmente apenas aritmética e branch interno, garantindo que cada valor vivo entre blocos seja armazenado no frame. O teste seguinte deverá executar código gerado de verdade e comparar com o caminho C++ normal antes de ampliar a cobertura.


### Ajustes de validação do frame — 2026-11-11

- A primeira execução do Lint identificou apenas divergências de formatação no novo header e no teste; os trechos foram ajustados.
- O Lint & Format Check #101 passou no commit `ab8c60ba9b6546430014e09ec4f859f13b534906`.
- Após simplificar mais uma asserção longa do teste, o commit atual é `57b5aacf853f6ff6ed58e21ba24641b40407f34f`.
- No momento desta atualização, estão em execução:
  - Guest Frame Prototype Unit Tests #14: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43141340967
  - Lint & Format Check #102: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43141341027
- A aprovação do Lint #101 valida formatação do estado anterior ao último ajuste do teste; o resultado definitivo do commit atual depende dos workflows #14 e #102.


- O Lint #102 apontou mais uma quebra de linha na chamada usada pelo teste de payload truncado/excedente. O teste foi ajustado no commit `5fcb58abf9e5efaa29f8672df2cf61d91129fe5e`; os workflows disparados por esse commit são a validação definitiva pendente.


## 2026-11-11 — CI do frame resumível e correção final de formatação

- Guest Frame Prototype Unit Tests #15 passou: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43141460692
  - Commit validado: `5fcb58abf9e5efaa29f8672df2cf61d91129fe5e`.
  - O build dos testes e a execução dos testes de frame terminaram com sucesso.
- Lint & Format Check #104 e #105 falharam no mesmo ponto: formatação da chamada `ResumableFrame::Deserialize` em `tests/unit/ppc/resumable_frame_test.cpp:134`. Não foi reportado erro de compilação/teste nessa execução; o bloqueio dos dois runs foi o clang-format 20.1.8.
- Correção aplicada no teste, movendo a quebra de linha para depois do operador de atribuição:
  `const auto trailing_result =`
  seguida da chamada `ResumableFrame::Deserialize(...)`.
- Commit da correção: `83384fc508847f533dce25089e6f4409e20642cc`.
- Aguardar os novos workflows de lint e Guest Frame Unit Tests disparados por esse commit. Ainda não declarar o Lint aprovado até ver o resultado.
- Limite técnico permanece: os testes verdes #15 exercitam o frame versionado e uma fixture manual C++, não código PPC emitido por `FunctionNode`, nem restauração real entre processos. Não é Save State completo e não deve ser integrado ao APK funcional do RECVX ainda.
- Próxima etapa após CI verde: estudar e prototipar, em modo opt-in isolado, a emissão de blocos básicos como estados explícitos para uma função PPC sintética, sem alterar o codegen padrão/runtime nem o jogo funcional.


## 2026-11-12 — Nova tentativa de formatação do teste ResumableFrame

- O Lint #106 e #107 continuou falhando exclusivamente em `tests/unit/ppc/resumable_frame_test.cpp:134`: clang-format 20.1.8 rejeitou a quebra de linha após o operador de atribuição.
- A chamada foi revertida para a disposição em que a atribuição e o início da chamada ficam na mesma linha, com o argumento em linha seguinte.
- Correção enviada no commit `8a0a1dc7ac68ab5a64a4d8114762ab42fa13859a`.
- Novas validações disparadas:
  - Guest Frame Prototype Unit Tests #17: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43143579237
  - Lint & Format Check #108: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43143579218
- No momento desta atualização, ambos estavam na fila. Não declarar lint/testes #17 aprovados até consultar as conclusões.
- O Guest Frame Unit Tests #15 havia passado no commit anterior `5fcb58ab...`; essa aprovação não valida automaticamente o commit novo.


## 2026-11-12 — Correção estrutural da chamada span no teste ResumableFrame

- O Lint & Format Check #109 confirmou que a disposição anterior ainda falhava no clang-format 20.1.8, na linha 134 de `tests/unit/ppc/resumable_frame_test.cpp`. A falha era exclusivamente de formatação.
- Para evitar novas tentativas de quebra da mesma chamada, o teste agora declara primeiro `const std::span<const uint8_t> trailing_span(trailing_data);` e passa essa variável a `ResumableFrame::Deserialize(trailing_span)`.
- Commit da alteração: `cd141103499e2e797fdeb36f0478c9d1c131d4a8`.
- Novas validações do commit:
  - Guest Frame Prototype Unit Tests #18: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43144342660
  - Lint & Format Check #110: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43144342639
- As duas execuções estavam em andamento na última consulta. Consultar seus resultados antes de considerar o ajuste validado.
- O run #17 ainda estava em andamento e pertence ao commit anterior; não substitui a validação do commit #18.
- O frame continua sendo um protótipo parcial, e a fixture de retomada continua manual em C++: ainda não há emissão PPC resumível real nem Save State completo. Não alterar o caminho padrão nem integrar ao APK funcional do RECVX nesta fase.


## 2026-11-12 — Primeiro executor resumível de um subconjunto real de instruções PPC

### Implementação isolada
- Criado `include/rex/ppc/resumable_program.h`, com `ResumableProgram::Compile()` e `Run()`.
- O compilador experimental recebe palavras de instrução PPC em ordem arquitetural e as valida antes de criar uma sequência interna de operações com endereços guest explícitos.
- Subconjunto implementado:
  - `addi`: escreve GPR usando o valor zero especial quando RA=0 e imediato signed de 16 bits.
  - `bdnz`: decrementa CTR e escolhe o endereço de destino relativo ou o próximo PC.
  - `blr`: termina a execução resumível.
- Instruções não suportadas, branches absolutos/com link, variantes de branch diferentes de bdnz e destinos fora da função são rejeitados; não são aproximados nem silenciosamente ignorados.
- `Run(frame, instruction_budget)` executa um número limitado de instruções e retorna `Yielded`, `Completed` ou `Invalid`. O PC guest e os registradores usados permanecem no `ResumableFrame`.
- O programa é reconstruível a partir do código guest; o snapshot persiste o frame, não ponteiros host nem o vetor interno de operações.
- Esta via é independente e experimental. Ainda **não está conectada a `FunctionNode::emitCpp()`, ao dispatcher nem ao runtime normal**. É um executor de um subconjunto PPC, não ainda um gerador de código C++ resumível nem um Save State completo.

### Teste de execução PPC e retomada
- Expandido `tests/unit/ppc/resumable_frame_test.cpp` com um pequeno programa PPC codificado em palavras reais:
  - `addi r3, r0, 0`
  - `addi r3, r3, 1`
  - `bdnz` voltando à segunda instrução
  - `blr`
- O teste executa duas instruções, serializa o frame, destrói o objeto original, reconstrói o frame dos bytes e continua em quanta até o retorno. O resultado esperado é GPR3=5 e CTR=0.
- Outro teste verifica a rejeição de instrução não suportada e branch para fora da função.
- O workflow Guest Frame foi atualizado para monitorar `include/rex/ppc/resumable_program.h`.

### Commits e CI
- Novo executor: `057995237458c892084fbd105e3745a643516f19`.
- Inclusão explícita de `<cstddef>`: `495f6c04aeee03faea65c5c91af48ba3d08e34c0`.
- Teste PPC inicial: `02016c454ea68e409ede72f34eda2926b4aa7219`.
- Ajustes de includes e filtro do workflow: `e25531ebfa711527998708dc862a3011cb1cd3a0`.
- Ajustes de formatação e asserções: `b02805d98ed763b18406be28dcacb4f89d434935`, `3c635daabb4e4a8635ca10611ac6779824b91d2a` e `5dd23da8057595a29d950df2e336b161fae09ccf`.
- Lint & Format Check #119 passou no commit final acima: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43146012721
- Guest Frame Prototype Unit Tests #24 ainda estava em andamento na última consulta: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43146012663
- Não considerar a execução PPC validada até o workflow #24 compilar e executar os testes com sucesso.

### Próxima etapa após CI verde
1. Confirmar o workflow #24.
2. Adicionar casos de teste para imediato negativo, CTR=1 e orçamento de execução zero.
3. Só depois planejar a ponte opt-in que extrai instruções dos blocos reais de uma `FunctionNode`; manter o codegen padrão intacto.
4. Expandir instruções e fluxo de chamadas em passos pequenos, com rejeição explícita de operações sem semântica resumível.
5. Não integrar ao Resident Evil até haver equivalência PPC suficiente e teste de restauração em processo separado.

### Limites que permanecem
- O executor atual cobre apenas `addi`, `bdnz` e `blr`.
- A retomada do teste ocorre no mesmo processo, após reconstruir o frame a partir dos bytes; ainda não prova encerramento/reabertura do aplicativo.
- Não inclui FPR, VMX, memória guest, scheduler, threads, estado de kernel, áudio, vídeo, imports, chamadas PPC aninhadas, SEH ou callbacks.
- O caminho normal do SDK e o APK funcional do RECVX não foram modificados.


## 2026-11-12 — CI verde e ampliação dos testes de limites do executor PPC

- Verificadas as execuções do commit `61c8575cb7bda872adb482ad61cec81e0264cfd9`:
  - Guest Frame Prototype Unit Tests #24: **success** — https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43146012663
  - Lint & Format Check #120: **success** — https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43146171723
- Adicionados testes para:
  - `addi` com imediato negativo, confirmando extensão de sinal.
  - `CTR=1`, confirmando que `bdnz` decrementa CTR a zero e segue para a próxima instrução.
  - orçamento de execução igual a zero, confirmando que o executor retorna `Yielded` sem alterar PC nem CTR.
- Os testes foram adicionados em `tests/unit/ppc/resumable_frame_test.cpp`.
- Commits dos testes:
  - Primeiro conjunto: `bf086f2ce28e397bdfa0308a7568e243545f3cad`.
  - Correção do deslocamento relativo do branch no caso CTR=1: `3e35f1cb36729335257eebc8022bd8510da26eb5`.
- As novas alterações ainda precisam passar pelos workflows de Guest Frame Unit Tests e Lint; não declarar os novos casos aprovados até a conclusão dessas execuções.
- Próximo passo após CI verde: estudar a extração de instruções dos blocos reais de `FunctionNode`, começando por um caminho opt-in e limitado, sem modificar o codegen padrão. O executor ainda não está integrado ao dispatcher/runtime e não constitui Save State completo.
- O APK funcional do Resident Evil permanece intocado. Não integrar esta experiência ao jogo até demonstrar equivalência com código PPC real e restauração em processo separado.


## 2026-11-12 — Adaptador experimental entre FunctionNode e executor resumível

### Auditoria do codegen real
- Confirmado em `include/rex/codegen/function_node.h` que `FunctionNode` expõe, após discovery/seal, os blocos, as instruções decodificadas, as chamadas, tail calls, jump tables, branches não resolvidos e metadados de exceção.
- Em `src/codegen/function_graph.cpp`, `emitCpp()` percorre os blocos, lê cada palavra PPC, faz `Disassemble()` e chama `DispatchInstruction()`. O codegen normal emite labels/gotos e chamadas C++; não é seguro suspender esse corpo nativo.
- A primeira ponte opt-in foi isolada em `include/rex/codegen/resumable_function.h`; não altera `FunctionNode::emitCpp()`, builders, dispatcher ou runtime.
  
### Contrato do adaptador
- `CompileResumableFunction(const FunctionNode&)` só aceita função selada, não-import, com um único bloco contíguo que começa na entrada e cobre exatamente o tamanho da função.
- Rejeita funções com chamadas diretas, tail calls, jump tables, branches não resolvidos, metadados de exceção, instruções ausentes/desordenadas ou instruções fora do subconjunto suportado.
- Extrai as palavras das instruções decodificadas (conversão big-endian já tratada pelo tipo `rex::be<uint32_t>`) e delega a validação PPC ao `ResumableProgram::Compile()`.
- O primeiro teste constrói uma `FunctionNode` sintética real com instruções `addi/addi/bdnz/blr`, sela o nó, passa pelo adaptador e executa em quanta. Isso valida a ponte entre a estrutura de função e o executor, mas não é ainda o código C++ gerado por `emitCpp()` nem equivalência com a execução normal.
  
### Commits e validação pendente
- Header adaptador: `c56103b704982e34b34373649b412d51e723cd3d`.
- Teste com FunctionNode: `133524b9afe78bc30f99eb07eaa0b8acb1558308`.
- Workflow atualizado para monitorar o novo header: `535fe9cfed143ccf08877282c94aecc67b7a883a`.
- Workflows observados:
  - Guest Frame Prototype Unit Tests #28: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150203320 — em andamento na última consulta.
  - Lint & Format Check #126: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150203245 — em andamento na última consulta.
  - Lint & Format Check #125 (header isolado): https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150180969 — em andamento na última consulta.
- Não considerar o adaptador validado até os workflows #28 e #126 terminarem com sucesso.
  
### Limites e próximo passo
- O adaptador aceita apenas funções muito simples de bloco único; a maioria das funções reais do jogo será rejeitada por enquanto.
- O executor ainda suporta somente `addi`, `bdnz` e `blr`; não foi conectado ao codegen C++ nem ao runtime.
- Próximo passo após CI verde: adicionar testes de rejeição para funções com múltiplos blocos e chamadas, e verificar o comportamento de endereço/intervalo com casos inválidos. Depois estudar uma comparação de referência com o codegen normal em fixture pequena.
- Não integrar ao APK funcional do Resident Evil. Save State completo e restauração em outro processo ainda não existem.


### Ajustes de formatação e estado posterior — 2026-11-12
- Os primeiros Lints do adaptador (#125–#128) falharam somente por clang-format 20.1.8 nos arquivos novos e no teste; os logs não apontaram ainda falha de compilação do adaptador.
- Simplificadas as condições e o cálculo de endereço no header; reduzido o nome do caso de teste e fatorado o tamanho da função para variáveis locais.
- Commits de formatação:
  - Header: `41f74cf590a10409040753e6c5fda9ef11c05bf5`.
  - Teste: `b68b4fa1d42e32821f87beeb52406251b2a5375b`.
- Validações mais recentes observadas:
  - Guest Frame Prototype Unit Tests #30: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150314392 — em andamento na consulta.
  - Lint & Format Check #129: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150314384 — em andamento na consulta.
- Execuções anteriores de teste #28 e #29 também apareciam em andamento; verificar o resultado mais recente antes de considerar o adaptador validado.
- Não há ainda confirmação de equivalência com `emitCpp()`; este adaptador apenas extrai um subconjunto restrito de instruções decodificadas de um `FunctionNode` selado.


### Resultado atualizado do CI — 2026-11-12
- Lint & Format Check #130 passou no commit `b68b4fa1d42e32821f87beeb52406251b2a5375b`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150320605
- O lint confirmou formatação C/C++, pins Vulkan e testes de scripts.
- Guest Frame Prototype Unit Tests #31, mesmo commit, ainda estava em execução na última consulta: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43150320660
- O job ainda estava no checkout do repositório/submódulos; compilação e CTest ainda não haviam começado. A ponte FunctionNode só estará validada após o #31 concluir com sucesso.


## 2026-11-12 — Correção do include do adaptador FunctionNode

- Guest Frame Prototype Unit Tests #31 falhou durante **Build unit tests**, antes de executar CTest.
- O erro confirmado foi: `include/rex/codegen/resumable_function.h:11:10: fatal error: 'rex/codegen/ppc/instruction.h' file not found`.
- A inspeção do repositório confirmou que a definição de `rex::codegen::ppc::Instruction` está em `src/codegen/ppc/instruction.h`, não em `include/rex/codegen/ppc/instruction.h`. O alvo `unit_tests` inclui `${PROJECT_SOURCE_DIR}/src`.
- Corrigido o include do adaptador para `"codegen/ppc/instruction.h"`, usando a raiz `src` já presente nos diretórios de include do teste.
- Commit da correção: `495a203d6ad49deb5dec616e88c20216ac57607f`.
- Próximo passo: aguardar e verificar o novo Guest Frame Prototype Unit Tests e o Lint disparados por esse commit. A falha anterior foi de compilação do include; os testes do adaptador ainda não foram executados.
- O adaptador continua experimental e restrito. Nenhuma alteração foi feita no codegen normal, runtime ou APK funcional do Resident Evil; não é Save State completo.


## 2026-11-12 — CI verde do adaptador e testes negativos do FunctionNode

- A correção do include foi validada:
  - Guest Frame Prototype Unit Tests: **success** — https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43216681156
  - Lint & Format Check: **success** — https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43216681180
- Adicionados dois testes negativos em `tests/unit/ppc/resumable_frame_test.cpp`:
  - Função selada com dois blocos contíguos deve ser rejeitada pelo adaptador.
  - Função selada com uma chamada resolvida/registrada no grafo deve ser rejeitada.
- Os testes usam instruções PPC suportadas para isolar a validação estrutural do FunctionNode; não tentam executar funções multi-bloco ou com chamadas.
- Commit dos testes: `4faf262a9768f1dd57ea4310ba8d2f2ceec92e9a`.
- O workflow já monitora `tests/unit/ppc/resumable_frame_test.cpp`; a nova execução de Guest Frame e o Lint precisam terminar antes de declarar os casos aprovados.
- Próxima etapa após CI verde: testar entradas inválidas de instruções/endereço e, depois, planejar comparação de referência entre o adaptador/executor e o codegen normal em fixture controlada. O adaptador ainda não executa o C++ emitido por `FunctionNode::emitCpp()`.
- Nenhuma alteração no dispatcher, runtime ou APK funcional do Resident Evil. Isto continua sendo um protótipo parcial, não Save State completo.


## 2026-11-12 — Revisão dos testes negativos e correção do lint

- O Lint & Format Check do commit 4faf262a9768f1dd57ea4310ba8d2f2ceec92e9a falhou apenas em duas posições de formatação em tests/unit/ppc/resumable_frame_test.cpp (linhas próximas de 305 e 332); não foi uma falha de compilação.
- Corrigida a quebra/alinhamento de FunctionNode::discover() e removida a linha em branco excedente. Commits:
  - ca1b394e4205edd80832cdd357d5cad432b4a87b (também renomeia o caso para “rejects call edges”).
  - 5f1eca42aba45fe774a8441d88c3550ebcbc7fbb (ajuste final de alinhamento para clang-format).
- Revisão semântica: o teste de chamadas usa CallTarget::unresolved(...), registrado como uma CallEdge por FunctionGraph::addCallToFunction(). Portanto, ele comprova que o adaptador rejeita uma função que possui uma aresta de chamada registrada, mas não comprova especificamente uma chamada resolvida. O nome do teste e esta documentação foram ajustados para não exagerar o que foi validado.
- Guest Frame Prototype Unit Tests do commit anterior 4faf262... ainda estava em compilação na última consulta: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43260371990. Esse run não inclui as correções posteriores; aguardar a execução disparada pelo commit 5f1eca42... para validar o estado atual.
- Lint anterior falhou somente por clang-format: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43260372016. O commit 5f1eca42... contém a correção, mas o novo lint ainda precisa ser confirmado.
- O adaptador permanece experimental, limitado a funções de bloco único sem chamadas, e não executa o C++ emitido por FunctionNode::emitCpp(). Nenhuma alteração no runtime, dispatcher ou APK funcional do Resident Evil.


## 2026-11-12 — Unit tests verdes e novo ajuste de clang-format

- Guest Frame Prototype Unit Tests #33 (commit 4faf262...) concluiu com sucesso: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43260371990.
- Guest Frame Prototype Unit Tests #34 (commit ca1b394...) concluiu com sucesso: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43260993663.
- Guest Frame Prototype Unit Tests #35 (commit 5f1eca4...) concluiu com sucesso: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43261069290. Os dois testes negativos foram compilados e executados com sucesso.
- O Lint #139 revelou que o alinhamento de FunctionNode::discover() ainda não correspondia ao clang-format 20.1.8: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43261145200. A mensagem apontou apenas a chamada em tests/unit/ppc/resumable_frame_test.cpp, linhas 331–332.
- Corrigido o alinhamento colocando o primeiro argumento na mesma linha da chamada. Commit: 452f89320683c4a6cc1c64249f5fcfb88872831e.
- Guest Frame Prototype Unit Tests #36, no commit 452f893..., foi disparado e estava queued na última consulta: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43262590249. A validação do lint para esse commit ainda precisa ser confirmada.
- O teste de chamada usa CallTarget::unresolved() como CallEdge registrada; comprova rejeição de aresta de chamada, não de chamada resolvida. O teste foi nomeado “rejects call edges” para refletir isso.
- Nenhuma mudança no codegen normal, dispatcher, runtime ou APK do Resident Evil. O adaptador segue experimental e restrito; Save State completo ainda não existe.


## 2026-11-12 — Correção final do alinhamento do teste multi-bloco

- O Lint #140 mostrou que o clang-format 20.1.8 espera o primeiro argumento de FunctionNode::discover() agrupado na mesma linha até o limite de coluna e os argumentos seguintes alinhados como continuação. O ajuste anterior ainda deixava a quebra no lugar errado.
- Aplicada a forma de chamada com o primeiro argumento agregado na primeira linha e std::move(decoded) alinhado como argumento seguinte.
- Commit de código: 4597e071feb750ba8552219024ba3c151b0e9a84.
- Workflows desse commit:
  - Guest Frame Prototype Unit Tests #37: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43262855538 — em andamento na última consulta.
  - Lint & Format Check #142: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43262855562 — em andamento na última consulta.
- O teste unitário #35 já havia passado, validando compilação e execução dos dois testes negativos; #37 valida o ajuste de formatação mais recente.
- Nenhuma mudança no runtime, dispatcher, codegen normal ou APK funcional do Resident Evil. O adaptador segue experimental e Save State completo ainda não existe.

## 2026-11-12 — Guest Frame Unit Tests #37 aprovado e validação de limites de endereço

- Guest Frame Prototype Unit Tests #37 concluiu com sucesso no commit `4597e071feb750ba8552219024ba3c151b0e9a84`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43262855538
- O job completou checkout, configuração C++23, build dos testes unitários e execução do CTest. O Lint & Format Check #142 também passou: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43262855562
- Revisão do compilador experimental encontrou uma condição de limite excessivamente restritiva: o cálculo anterior rejeitava até uma função de uma única instrução em `0xFFFFFFFC`, embora esse seja o último endereço PPC de instrução alinhado representável em 32 bits.
- Corrigida a validação para verificar o endereço da última instrução (`words.size() - 1`) em vez de exigir que o endereço exclusivo final caiba em `uint32_t`. O cálculo do fim da função para validar branches continua em `int64_t`.
- Commit da correção: `cb9fedb6154d853520a3c2c92e963c76ffb560ef`.
- Adicionado teste que:
  - rejeita endereço zero;
  - rejeita endereço base desalinhado;
  - rejeita duas instruções que ultrapassariam o espaço de endereços guest;
  - aceita uma instrução `blr` em `0xFFFFFFFC` e a executa até `Completed`.
- Commit do teste: `b5de52c403736955792662d3fa86a41c17ca4bf5`.
- Os novos workflows disparados por esses commits ainda precisam terminar. Não considerar os novos casos aprovados até verificar Guest Frame Unit Tests e Lint.
- Próxima etapa após CI verde: construir uma fixture de referência controlada para comparar resultados de execução do subconjunto resumível com semântica PPC de referência. Isso ainda não equivale a executar o C++ emitido por `FunctionNode::emitCpp()`; qualquer comparação deve explicitar esse limite. Não integrar ao runtime nem ao APK funcional do Resident Evil nesta fase.
- Estado do projeto: o adaptador e executor continuam experimentais e limitados a `addi`, `bdnz` e `blr`. Não existe ainda Save State completo, persistência de memória guest/threads/kernel ou restauração após fechar e reabrir o aplicativo.

### Ajuste de formatação do teste de limites — 2026-11-12
- Lint & Format Check #145 falhou somente no clang-format do novo caso de teste de limites, em `tests/unit/ppc/resumable_frame_test.cpp`; não chegou a indicar falha de compilação ou semântica.
- A saída do clang-format apontou uma linha em branco excedente e a assinatura do `TEST_CASE`, que cabe em uma única linha.
- Corrigido o espaçamento e colocada a assinatura do teste em uma linha.
- Commit da correção: `f068f6bb8ce69567a3d410e9ef3f992d97b92c9f`.
- Lint #144 do header `resumable_program.h` passou: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43265411067
- Guest Frame Unit Tests #38 (commit `cb9fedb...`) e #39 (commit `b5de52c...`) ainda estavam em andamento na última consulta. O lint #145 pertence ao commit anterior e foi substituído pela correção acima.
- Verificar as novas execuções disparadas pelo commit `f068f6b...` antes de declarar a etapa validada.

### Validação do ajuste de formatação — 2026-11-12
- Lint & Format Check #147 passou no commit `f068f6bb8ce69567a3d410e9ef3f992d97b92c9f`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43265585017
- O Guest Frame Prototype Unit Tests #40, no mesmo commit, está em andamento. Na última consulta, o job ainda executava o checkout do código e submódulos; build e CTest ainda não começaram: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43265585024
- O #40 é a validação que deve ser usada para confirmar os testes de limites de endereço e a correção de formatação no mesmo commit. Os runs #38 e #39 pertencem a commits anteriores e não substituem essa confirmação.

### Guest Frame Unit Tests #40 aprovado — 2026-11-12
- Guest Frame Prototype Unit Tests #40 passou no commit `f068f6bb8ce69567a3d410e9ef3f992d97b92c9f`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43265585024
- Todos os passos do job passaram: checkout e submódulos, dependências, verificação C++23, configuração CMake, build dos testes unitários e execução do CTest.
- Lint & Format Check #147 também passou no mesmo commit: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43265585017
- Portanto, a correção de limite de endereços e os testes associados estão validados pelo CI.
- Próxima etapa: desenhar uma comparação diferencial de semântica PPC em uma fixture pequena. O lado de referência deve calcular o resultado esperado conforme as regras da ISA (ou usar um interpretador de referência independente); o lado experimental executará o `ResumableProgram`. Comparar estado observável após execução completa e após yield/serialize/deserialize/resume.
- Limite importante: essa fixture ainda não provará equivalência com o C++ real emitido por `FunctionNode::emitCpp()`. Para isso será necessário executar o código gerado com contexto/ambiente real ou um harness equivalente e comparar resultados. Não declarar equivalência do codegen nem integrar ao runtime/APK com base apenas na fixture.

### Comparação diferencial inicial do executor PPC — 2026-11-12
- Após Guest Frame Unit Tests #40 e Lint #147 passarem, foi adicionada uma referência independente, escrita diretamente sobre as palavras PPC da fixture, para validar o subconjunto suportado.
- Novo teste: `Resumable PPC execution matches independent reference across save points`.
- A fixture contém `addi r3,r0,-2`, `addi r3,r3,3`, `bdnz` relativo e `blr`; testa CTR inicial 1, 2, 5 e 17.
- O caminho de referência interpreta as palavras PPC sem chamar `ResumableProgram`; o caminho resumível executa em orçamento de duas instruções, serializa/desserializa em cada yield e compara GPRs, CTR e PC final.
- Commit do teste: `635fdf3772eb807f81c1905384ebd481d1a0fe2d`.
- Este é um teste diferencial de uma fixture pequena e de um subconjunto PPC restrito. Não compara com o C++ emitido por `FunctionNode::emitCpp()`, nem valida calls, FPR/VMX, memória guest, threads ou o runtime do jogo.
- Os workflows de CI disparados pelo commit ainda precisam passar antes de considerar a fixture validada.


### Comparação diferencial PPC aprovada pela CI — 2026-11-12
- Guest Frame Prototype Unit Tests #41 passou no commit `635fdf3772eb807f81c1905384ebd481d1a0fe2d`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43267780380
- Lint & Format Check #151 também passou no mesmo commit: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43267780491
- Lint & Format Check #152 passou no commit de documentação anterior `ab4484aa11ef082fc18977635dcc3ee4690c3565`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43267801436
- A fixture diferencial está validada pela CI: para o subconjunto `addi`, `bdnz` e `blr`, a execução resumível com yields e serialize/deserialize produz os mesmos GPRs, CTR e PC final que o interpretador de referência independente, nos valores de CTR testados (1, 2, 5 e 17).
- Limite: a referência ainda é um interpretador independente da fixture, não o C++ AOT produzido por `FunctionNode::emitCpp()`. Não prova equivalência geral do codegen nem compatibilidade com o jogo real.
- Próxima investigação: criar um harness que capture e execute uma função C++ efetivamente emitida por `FunctionNode::emitCpp()`, com os mesmos registradores/contexto guest e helpers necessários, e comparar o estado observável com a semântica PPC de referência. Antes de implementar, mapear a assinatura/macros `REX_FUNC`, o tipo de contexto usado pelo código emitido e as dependências de helpers; não alterar o caminho de produção.
- Estado: adaptador e executor permanecem opt-in e experimentais. Não há Save State completo, snapshot da memória guest, persistência/restauração de threads/kernel, nem integração com o runtime ou APK funcional do Resident Evil.


### Auditoria do contrato do C++ AOT — 2026-11-12
- Revisados `src/codegen/function_graph.cpp`, `src/codegen/builders/builder_context.h`, `src/codegen/builders/context.cpp`, `src/codegen/builders/control_flow.cpp`, `include/rex/ppc/context.h` e `include/rex/ppc/func.h`.
- `FunctionNode::emitCpp()` gera uma função C++ normal com assinatura PPC `PPCFunc(PPCContext& ctx, uint8_t* base)`, emite o prologue e traduz o fluxo interno em labels/`goto` e retornos C++.
- O codegen pode localizar registradores em variáveis C++ locais. `BuilderContext::r/f/v/cr/ctr/xer` seleciona entre `ctx.*` e locais conforme `RecompilerConfig`; `emitCpp()` declara esses locais antes do corpo. Também podem existir temporários locais (`temp`, `vTemp`, `env`, `ea`).
- As chamadas de função PPC resolvidas são emitidas como chamadas C++ diretas `nome(ctx, base)`; `blr` vira `return;`. Logo, um snapshot de `PPCContext` sozinho não captura os valores vivos que permanecem apenas nas variáveis locais nem a pilha de chamadas nativa.
- `PPCContext` contém GPRs, FPRs, VMX, LR, CTR, XER, CR, FPSCR e flags auxiliares; porém isso não significa que todo o estado em execução esteja sempre sincronizado nele. A localização de registradores depende da configuração do codegen.
- Conclusão arquitetural: o próximo passo não é tentar suspender arbitrariamente a função C++ atual. É definir pontos de continuação/safepoints nos quais todos os valores vivos sejam explicitamente materializados em um frame serializável (ou manter esses valores no contexto de forma comprovada), com PC guest inequívoco. Chamadas precisam de uma política explícita de checkpoint/retorno; temporários e estado de FPU/VMX também precisam ser cobertos.
- Próximo experimento seguro: construir um teste de codegen que gere uma função sintética por `FunctionNode::emitCpp()`, confirme o padrão de variáveis locais, labels e chamadas, e então desenvolver uma transformação opt-in para um subconjunto sem chamadas e sem temporários complexos. O teste de texto é somente a primeira etapa; a meta seguinte é compilar e executar a função gerada em harness, não apenas comparar strings.
- Nenhuma alteração foi feita em `emitCpp()`, nos builders, dispatcher, runtime ou APK. O jogo funcional permanece intacto. Save State completo continua não implementado.


### Primeiro teste do C++ emitido por FunctionNode — 2026-11-12
- Adicionado em `tests/unit/ppc/resumable_frame_test.cpp` o teste `FunctionNode emits synthetic PPC function with materialized locals`.
- O teste monta uma seção executável sintética via `TestModule`/`BinaryView::fromModule()`, cria e sela um `FunctionNode`, e chama o `FunctionNode::emitCpp()` real.
- Fixture: `addi r11, r0, 1`; `blr`. A configuração ativa `nonArgumentRegistersAsLocalVariables`, de modo que o teste verifica a saída emitida para `PPCRegister r11{}`, a atribuição `r11.s64 = 1;`, assinatura `DEFINE_REX_FUNC(...)` e retorno C++.
- Este é o primeiro teste que passa pelo emissor AOT real. Porém, por enquanto verifica a estrutura textual do C++ emitido; ainda não compila nem executa dinamicamente essa função gerada, portanto não demonstra equivalência de execução ou pontos de continuação.
- CI para o commit final do teste `1cb37cb1704969da6fbf96a47a71797f6f98db36`: Guest Frame Prototype Unit Tests #43 https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273413465 e Lint & Format Check #156 https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273413392. Ambos estavam em execução no momento da atualização.
- Também há execuções anteriores do commit intermediário `20eebd1bb907979090f4c402a5ff2a85ff92b746` (#42/#155); a validação de referência é a execução do commit final #43/#156.
- Próximo passo, condicionado à CI: transformar a saída em uma unidade C++ de teste compilável com os headers/macros de PPC, fornecer stubs controlados para dependências e invocar a função gerada com um `PPCContext` real. Só então comparar estado antes/depois com a referência independente.
- Nenhuma integração ao runtime/dispatcher foi feita; nenhum APK foi alterado. O protótipo continua experimental e o Save State completo não está implementado.


### Correção de formatação do teste AOT — 2026-11-12
- O primeiro Lint #156/#157 apontou apenas duas diferenças de clang-format no novo teste; nenhuma falha semântica foi reportada por esses jobs.
- Corrigidas as duas posições em `tests/unit/ppc/resumable_frame_test.cpp` (linha em branco extra e quebra do primeiro CHECK).
- Commit da correção: `22eb6dd586d56eb16e1365b8c62756bfa2f27aa8`.
- Lint & Format Check #158 passou: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273682476
- Guest Frame Prototype Unit Tests #44 foi iniciado para esse commit: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273682433. Na última consulta, ainda estava em checkout de código/submódulos; o build e CTest ainda não haviam começado. Os jobs #42/#43, dos commits intermediários, também continuavam sem conclusão. Portanto, o novo teste de emissão AOT ainda não está validado pela CI.
- A documentação deve considerar como código mais recente o commit `22eb6dd586d56eb16e1365b8c62756bfa2f27aa8`; a atualização deste registro é posterior.


### Guest Frame Unit Tests #44 aprovado e revisão do harness AOT — 2026-11-12

- Guest Frame Prototype Unit Tests #44 concluiu com sucesso no commit `22eb6dd586d56eb16e1365b8c62756bfa2f27aa8`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273682433
- Lint & Format Check #158 também passou no mesmo commit: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273682476
- O Lint & Format Check #159 passou no commit de documentação anterior `3495efd28e22248a54ce6c69a818a07401462ef4`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43273840119
- Portanto, o teste textual que chama o `FunctionNode::emitCpp()` real está compilado e executado pela suíte. Ele ainda verifica somente o texto gerado, não compila/executa a função C++ emitida.
- Revisão do harness:
  - `PPCFunc` é um ponteiro para função com assinatura `void(PPCContext& ctx, uint8_t* base)`, declarado em `include/rex/ppc/func.h`.
  - `PPCContext` e os aliases de registradores estão em `include/rex/ppc/context.h`.
  - O emissor gera `DEFINE_REX_FUNC(nome)`; a definição/expansão exata desse macro e o conjunto completo de includes/helpers necessários ainda precisam ser localizados antes de montar uma unidade de compilação. Não presumir que `func.h` sozinho seja suficiente.
  - O fixture atual usa `addi r11, r0, 1; blr` e ativa `nonArgumentRegistersAsLocalVariables`; o valor de r11 é mantido em variável local C++, e não deve ser confundido com `ctx.r11`.
  - `tests/unit/CMakeLists.txt` já liga `unit_tests` a `rexcodegen` e `rexruntime`, mas isso não resolve automaticamente a compilação de uma string gerada em tempo de execução. A estratégia de geração pré-build ou fonte fixture gerada precisa ser escolhida com cuidado.
- Próximo passo: rastrear a definição de `DEFINE_REX_FUNC`, prologue e dependências de helpers/macros nos templates e fontes do SDK; então criar uma fixture C++ gerada em etapa explícita do build (ou outro mecanismo reprodutível), compilá-la como parte do alvo de testes e invocá-la com `PPCContext` real. Comparar os registradores observáveis com a semântica PPC de referência.
- Não alterar o codegen de produção, dispatcher, runtime ou APK funcional do Resident Evil nesta etapa. Não suspender função C++ arbitrariamente. Save State completo continua não implementado.


### Primeiro fixture PPC para executar C++ AOT gerado — 2026-11-13

- A definição de `DEFINE_REX_FUNC` foi localizada em `resources/templates/codegen/pch_h.inja`; ela pertence ao PCH gerado do projeto, não a `include/rex/ppc/func.h`. O header PCH também define `REX_FUNC_PROLOGUE`, macros de memória e helpers usados pelo código emitido.
- `include/rex/ppc/func.h` define a assinatura `PPCFunc` como `void(PPCContext&, uint8_t*)` e fornece `REX_FUNC`/`REX_EXTERN`. `include/rex/ppc.h` agrega o contexto e as APIs PPC.
- Foi encontrado um pipeline reutilizável em `tests/ppc`: compila assembly PPC, linka binário, chama `rexglue recompile-tests`, gera `ppc_test_functions.cpp` com `FunctionNode::emitCpp()`, compila esse C++ gerado e executa as funções com um `PPCContext` real. O pipeline já compara registradores de saída com valores esperados.
- Criada a fixture `tests/ppc/asm/seq_aot_runtime.s`:
  - entrada: r3 = 0x29;
  - instrução: `addi r11, r3, 7`;
  - retorno: `blr`;
  - saídas esperadas: r3 permanece 0x29 e r11 = 0x30.
- A fixture usa o fluxo real de recompilação PPC e é um passo mais forte que o teste textual anterior: se o pipeline passar, comprova que uma função emitida por `FunctionNode::emitCpp()` é compilada e executada e que o estado de registradores observado corresponde ao esperado nessa fixture simples.
- Atualizado `.github/workflows/guest-frame-prototype-tests.yml` para monitorar a fixture/pipeline, construir também o alvo `ppc_tests` e executar o caso `aot_runtime_addi` junto dos testes Guest Frame.
- Commits:
  - fixture assembly: `2ac65f42239cc89f25256c34a43cd8d28cd6f5bf`;
  - workflow: `10a5ec26958d5499455316c88469571fcd6444ef`.
- Status: a CI da nova fixture ainda precisa terminar. Não declarar execução AOT aprovada até o job compilar `ppc_tests` e o CTest executar o caso. O teste usa a configuração padrão do recompiler; ainda não testa materialização de registradores locais ativada, checkpoints/safepoints ou retomada.
- Próxima etapa após CI verde: criar comparação de referência independente para o resultado da função AOT e, depois, uma estratégia explícita de safepoints que materialize valores vivos antes de qualquer yield. Não conectar ao runtime do jogo nem alterar o APK funcional do Resident Evil.



### Primeira execução real do C++ AOT aprovada pela CI — 2026-11-13

- Guest Frame Prototype Unit Tests #45 passou no commit `10a5ec26958d5499455316c88469571fcd6444ef`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43405360892
- Lint & Format Check #162 passou no mesmo commit: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43405360814
- Lint & Format Check #163 também passou após a atualização de documentação no commit `ba14c652f69ed73fd0a48fdaab2c717803cbe71a`: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/43405386009
- O job #45 concluiu todas as etapas: checkout/submódulos, dependências, verificação C++23, configuração, build de `unit_tests` e `ppc_tests`, e execução CTest.
- Verificação do log: `ppc.seq.test_aot_runtime_addi` foi efetivamente executado e passou (1/1 teste; 0 falhas). O pipeline montou `ppc_test_functions.cpp` e `ppc_test_cases.cpp`, compilou os fontes gerados e linkou o executável `ppc_tests`.
- A fixture `seq_aot_runtime.s` inicializa r3=0x29, executa `addi r11,r3,7`, retorna com `blr`, e confirma r3=0x29 e r11=0x30. Portanto, o código emitido pelo `FunctionNode::emitCpp()` foi compilado e executado com `PPCContext` real para esse caso.
- Limite: o teste usa a configuração padrão de `RecompilerConfig`; não força GPRs locais nem valida checkpoint/yield/resume. É prova de execução AOT de uma fixture simples, não de Save State ou equivalência geral do jogo.

### Próxima investigação do codegen — 2026-11-13

- Revisado o caminho de configuração do pipeline em `src/rexglue/commands/test_recompiler.cpp`: atualmente ele cria `RecompilerConfig config;` com valores padrão e apenas define `outDirectoryPath`. Assim, a fixture AOT aprovada não exercita explicitamente `nonArgumentRegistersAsLocalVariables` nem as demais opções de localização.
- Revisados novamente `BuilderContext::r/f/v/cr/ctr/xer` em `src/codegen/builders/context.cpp`: cada accessor pode retornar `ctx.*` ou uma variável C++ local e marca a variável em `RecompilerLocalVariables`. Os temporários `temp`, `vTemp`, `env` e `ea` também são locais.
- O pipeline de teste permite adicionar uma opção estritamente de teste para emitir uma segunda variante com registradores localizados, sem alterar a configuração padrão nem o runtime de produção. Essa variante deve confirmar primeiro a saída gerada e depois ser compilada/executada com uma fixture em que um valor local precise sobreviver até o retorno.
- Direção segura: acrescentar uma variante de teste AOT com localização de registradores explicitamente habilitada e comparar os resultados observáveis com a variante padrão. Em seguida, projetar um mecanismo opt-in de safepoint que faça spill explícito para um frame/contexto antes do yield. Não inserir yield arbitrário em funções C++ e não presumir que os locais podem ser restaurados pelo `PPCContext`.
- Não houve alteração no codegen de produção, dispatcher, runtime, APK do Resident Evil ou Pac-Man. O Save State continua experimental e incompleto; ainda faltam safepoints reais, estado FPR/VMX abrangente, memória guest, scheduler/threads/kernel e persistência segura.


### Variante AOT com GPR localizado — 2026-11-13

- O teste anterior confirmou execução AOT usando a configuração padrão, mas não exercitava GPRs em variáveis locais. Foi criada uma variante dedicada, isolada do restante da suíte.
- Em `src/rexglue/commands/test_recompiler.cpp`, somente para o stem `seq_aot_runtime`, o pipeline de teste ativa `config.nonArgumentRegistersAsLocalVariables = true`. Os demais arquivos de instruções continuam usando a configuração padrão.
- A fixture `tests/ppc/asm/seq_aot_runtime.s` foi alterada para:
  - receber r3 = 0x29;
  - executar `addi r11, r3, 7`, mantendo o resultado intermediário em r11 localizado;
  - executar `addi r3, r11, 0`, lendo o valor local e copiando-o para o registrador de saída observável;
  - retornar e validar r3 = 0x30.
- O teste CTest foi renomeado para `aot_runtime_localized_gpr` no workflow. A validação precisa confirmar que o C++ gerado declara e usa `PPCRegister r11{}`, compila e executa a função e produz r3 = 0x30.
- Commits da alteração:
  - configuração de teste: `d6ceed03a1639d7be4503b7a69a4323ed80fb9ef`;
  - fixture: `b3dc244534971a879af982ab72c4a9b298763718`;
  - filtro CTest do workflow: `4c158f6139de9c5611f84e0b8be6e61c1af112e6`.
- Status: CI desta variante ainda precisa terminar. Não declarar a execução localizada aprovada até confirmar o CTest `ppc.seq.test_aot_runtime_localized_gpr`.
- Escopo: isso valida o uso e a leitura de um GPR local dentro de uma chamada AOT completa. Não salva/restaura esse local em um ponto intermediário, não cria safepoint e não demonstra que o valor pode ser serializado. Nenhuma alteração no codegen de produção, runtime, dispatcher ou APK funcional.

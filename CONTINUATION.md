# CONTINUATION.md — ReXGlue SDK Android ARM64

Última atualização: 2026-11-10

## Objetivo

Registrar o andamento da adaptação do ReXGlue SDK para Android ARM64, com foco na criação de uma superfície Vulkan baseada na janela nativa Android (ANativeWindow), necessária para a execução dos projetos de recompilação no Android.

Repositório: https://github.com/joaovitoroutlok821-dot/rexglue-sdk
Branch de trabalho: `android-arm64-fiber-experiment`

## Estado atual

Foram adicionadas alterações iniciais para reconhecer Android ARM64 na configuração CMake e conectar a janela SDL3 à superfície Vulkan nativa do Android.

**Importante:** a integração ainda NÃO está validada por uma compilação completa do SDK para Android ARM64. Os commits listados abaixo registram alterações de código, não uma confirmação de build funcional.

## Alterações realizadas

As alterações abaixo foram gravadas diretamente na branch de trabalho:

1. **CMake raiz — suporte inicial à plataforma Android**
   - Commit: `94a4b9c425e081ef99e86041488648352d55fb24`
   - Adicionada uma ramificação `ANDROID` antes da ramificação genérica UNIX.
   - A configuração identifica o alvo como `android-arm64`, exige arquitetura ARM64 e define `REX_PLATFORM_ANDROID=1` e `REX_PLATFORM_LINUX=1`.
   - A ramificação Linux existente foi mantida separada.

2. **CMake da interface — seleção dos arquivos Android**
   - Commit: `d251402c5729ce6a3eb2b5cac54814197df4a3de`
   - Adicionado `surface_android.cpp` como fonte da plataforma Android.
   - A detecção e dependências X11/Wayland foram restringidas a UNIX que não seja Apple nem Android.
   - Adicionada a ligação com a biblioteca Android do NDK (`android`).

3. **Integração da janela SDL3**
   - Commit: `ecd019099d45c22bad4f242e97dab29c8a9f3c9c`
   - `window_sdl.cpp` inclui a implementação de superfície Android sob `REX_PLATFORM_ANDROID`.
   - A janela nativa é obtida pela propriedade SDL3 `SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER`, usando `SDL_GetPointerProperty`.
   - A superfície `AndroidNativeWindowSurface` é criada a partir do `SDL_Window*` e do `ANativeWindow*`.

4. **Declaração da superfície Android**
   - Commit: `2dacbc163ebde15490b4fc65e46b34d389d1ec87`
   - Criado `include/rex/ui/surface_android.h`.
   - A classe `AndroidNativeWindowSurface` deriva de `Surface`, expõe o tipo `kTypeIndex_AndroidNativeWindow` e fornece acesso ao `ANativeWindow*`.

5. **Implementação da superfície Android**
   - Commit: `3594d6b844b41431534047227f6be28492167a44`
   - Criado `src/ui/surface_android.cpp`.
   - A implementação adquire a referência do `ANativeWindow` no construtor e libera no destrutor.
   - As dimensões em pixels são obtidas pela janela SDL com `SDL_GetWindowSizeInPixels`.

## Integração Vulkan já existente no código

Na inspeção anterior do repositório, foram encontrados componentes que já contemplavam a superfície Android:

- `vulkan_presenter.cpp` já possui o caso `AndroidNativeWindowSurface` e chama `vkCreateAndroidSurfaceKHR`.
- A implementação da instância Vulkan já habilita `VK_KHR_android_surface` quando `VK_USE_PLATFORM_ANDROID_KHR` está definido.
- `surface.h` já contém o identificador `kTypeIndex_AndroidNativeWindow` e a flag correspondente.
- O SDL3 está fixado no submódulo no commit `147a8ee32dbf9ac02f3794964490687b6bbda1bc`; a propriedade Android usada acima existe nessa versão inspecionada.

Esses pontos foram identificados por leitura do código. Ainda precisam ser confirmados por compilação Android real.

## CI / GitHub Actions

Foi iniciado automaticamente o workflow **Lint & Format Check #26** para o commit `3594d6b844b41431534047227f6be28492167a44`.

Execução: https://github.com/joaovitoroutlok821-dot/rexglue-sdk/actions/runs/42930048030

Último estado observado durante a sessão de trabalho: `in_progress`. O resultado final ainda precisa ser consultado.

O workflow de teste Android ARM64 do Fiber mencionado anteriormente valida somente os arquivos do backend Fiber (`fiber_android_arm64.cpp` e seu assembly). Ele **não** comprova a compilação completa do SDK nem a integração Vulkan/superfície Android.

## Pendências — próxima sequência de trabalho

1. Consultar o resultado final do Lint & Format Check #26 e corrigir eventuais falhas.
2. Revisar o contrato completo da classe base `Surface` e confirmar assinaturas, visibilidade e ciclo de vida da superfície Android.
3. Verificar os includes SDL3 usados em `window_sdl.cpp` e garantir que a propriedade `SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER` esteja visível no build Android.
4. Confirmar que `VK_USE_PLATFORM_ANDROID_KHR` é definido para os alvos Android e que os headers Vulkan expõem `VkAndroidSurfaceCreateInfoKHR` e `vkCreateAndroidSurfaceKHR`.
5. Auditar outros blocos CMake condicionados apenas por `UNIX` para evitar que dependências Linux/X11/Wayland sejam aplicadas ao Android.
6. Revisar o tempo de vida de `SDL_Window*` e `ANativeWindow*` para evitar uso após destruição da janela.
7. Preparar ou adaptar um workflow GitHub Actions que configure e compile o SDK completo com Android NDK, ARM64 e dependências Vulkan necessárias.
8. Executar a compilação completa e corrigir erros em sequência.
9. Só depois de um build completo aprovado, integrar a versão validada do SDK ao projeto Resident Evil Code Veronica e testar a geração do APK.

## Restrições e critérios de validação

- O alvo é Android ARM64 (ABI `arm64-v8a`).
- O desenvolvimento e a validação devem poder ser feitos pelo GitHub Actions, pois o ambiente de desenvolvimento disponível é um telefone Android, sem computador.
- Não considerar a adaptação concluída apenas porque o CMake configura, o Lint passa ou o teste isolado do Fiber passa.
- Critério mínimo: compilação completa do SDK Android ARM64 incluindo a camada UI e o caminho de apresentação Vulkan Android.
- Critério posterior: APK integrado instala, abre a janela nativa, apresenta a imagem corretamente e mantém o ciclo de vida da superfície sem crash.

## Último ponto para retomar

Retomar pelo resultado do workflow #26. Em seguida, revisar os contratos e macros Android/Vulkan e montar a primeira compilação completa do SDK. Não repetir as cinco alterações já registradas acima sem antes identificar um erro concreto que exija modificá-las.

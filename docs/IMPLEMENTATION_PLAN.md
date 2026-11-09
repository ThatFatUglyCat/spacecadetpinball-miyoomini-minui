# Plano de Implementação — SpaceCadetPinball como core libretro para MinUI (Miyoo Mini)

> **Status:** rascunho v1 · **Data:** 2026-10-30
> **Objetivo em uma frase:** gerar um core libretro (`spacecadetpinball_libretro.so`) a partir da decompilação
> [`k4zmu2a/SpaceCadetPinball`](https://github.com/k4zmu2a/SpaceCadetPinball), empacotado como *pak* do MinUI para o
> Miyoo Mini / Mini Plus, **sem distribuir nenhum arquivo proprietário** — o usuário fornece o `PINBALL.DAT` dele.

Convenções deste documento:

- `[VERIFICAR]` = afirmação feita de memória ou inferida; confirme no código/documentação antes de depender dela.
- `[DECISÃO]` = ponto em que existe mais de um caminho razoável; há uma recomendação, mas o dono do projeto decide.
- Código e documentação **do repositório** devem ser em inglês (projeto público). Este plano está em PT-BR.
- Estimativas de esforço são ordens de grandeza para uma pessoa com experiência em C++, em tempo parcial.

---

## 1. Visão geral

### 1.1 Objetivos

1. Rodar o jogo **original** (lógica, física, regras e assets do usuário) no Miyoo Mini com o MinUI, a 640×480.
2. Integrar-se ao MinUI via `minarch` (menu de jogo, controles, volume, brilho, sleep).
3. Ser **legalmente limpo para publicar no GitHub**: nenhum asset, nenhum derivado de asset e nenhum build step que dependa de
   arquivos da Microsoft/Cinematronics/Maxis.
4. Ser reproduzível: qualquer pessoa clona, roda um comando em Docker e obtém o `.pak`.
5. (Meta posterior) Suportar save states / auto-resume do MinUI.

### 1.2 Não-objetivos (por ora)

- Outros dispositivos além de `miyoomini` (Mini e Mini Plus compartilham a plataforma). Outros aparelhos ficam para depois,
  se o core for portátil.
- Outras mesas (Skullduggery, Dragon's Keep). O upstream hoje foca na mesa Space Cadet.
- Multiplayer, editor de dados, localização, interface ImGui.
- Compatibilidade com RetroArch “completo” como alvo principal. O core deve funcionar nele (útil para desenvolver no desktop),
  mas o alvo de teste é o `minarch`.

### 1.3 Critérios de sucesso

| Marco | Critério |
|---|---|
| **MVP** | Pak carrega no MinUI, abre `PINBALL.DAT` fornecido pelo usuário, mostra a mesa a 640×480, flippers/plunger respondem, som de efeitos funciona, 60 fps estáveis, pontuação máxima salva. |
| **v1.0** | MVP + opções do core, música opcional, mensagens de erro amigáveis, CI verde, release com checksums, docs completas. |
| **v1.x** | Save states / auto-resume determinísticos. |

---

## 2. Fatos de partida e pontos a verificar

| # | Fato | Origem | Status |
|---|---|---|---|
| 1 | Upstream é licenciado sob **MIT**, usa C++11, depende de SDL2 e SDL2_mixer (e Dear ImGui para a UI). | README do upstream | Confirmado (MIT, C++11, SDL2/SDL2_mixer); ImGui citado nos “Plans” do README |
| 2 | Os assets **não** estão no repositório; o jogo lê os dados originais (Windows ou Full Tilt). | README do upstream | Confirmado |
| 3 | Já existem ports do upstream para hardware limitado: Wii, 3DS, DS, PS2, Dreamcast, Vita, Switch, Wii U, webOS, Emscripten. | README do upstream | Confirmado — **use como referência de como remover SDL/ImGui e tratar tempo/áudio** |
| 4 | Um pak do MinUI é uma pasta `.pak` com `launch.sh`; emuladores ficam em `Emus/<plataforma>/`. | `PAKS.md` do MinUI | Confirmado |
| 5 | O pak pode trazer o próprio core libretro (tipo 2) e manter integração total com o menu do MinUI. | `PAKS.md` | Confirmado |
| 6 | O MinUI só suporta **RGB565** e não implementa as APIs OpenGL do libretro. | `PAKS.md` (“Caveats”) | Confirmado — o jogo é software-rendered, sem impacto |
| 7 | O log do `minarch` e do core vai para `/.userdata/<plataforma>/logs/<TAG>.txt`. | `PAKS.md` | Confirmado |
| 8 | A toolchain de referência é a Docker `shauninman/union-miyoomini-toolchain` (`make shell`; expõe `CROSS_COMPILE` e `PATH`). | Repositório da toolchain | Confirmado. Um fork do LessUI menciona precisar de patch no Dockerfile por causa de repositórios Debian arquivados e publica imagens prontas em `ghcr.io/lessui-hq/union-<plataforma>-toolchain` `[VERIFICAR]` |
| 9 | O repositório do MinUI foi **arquivado** em nov/2025 (somente leitura). | Página do repo | Confirmado — pin de versão e documentação do que foi testado são essenciais |
| 10 | Não encontrei core libretro pronto do Space Cadet Pinball. | Busca web (não exaustiva) | **Repetir a busca** (GitHub, libretro-super, buildbot) antes de começar, para evitar retrabalho e conflito de nome |
| 11 | O Miyoo Mini tem tela 640×480 e CPU ARM Cortex-A7 com NEON/VFPv4, ~128 MB de RAM. | Memória | `[VERIFICAR]` no aparelho (`cat /proc/cpuinfo`, `free -m`) |
| 12 | Arquivo DAT começa com um cabeçalho identificável (suspeito: `PARTOUT(4.0)RESOURCE`). | Memória | `[VERIFICAR]` em `partman.cpp`; usar para validar o conteúdo em vez de confiar na extensão |

---

## 3. Licenciamento e política de arquivos proprietários

> Esta seção é informativa e **não é aconselhamento jurídico**. Para um projeto público, vale uma revisão rápida de um
> advogado familiarizado com software livre/engenharia reversa.

### 3.1 Inventário de licenças

| Componente | Licença | Uso no projeto | Obrigações |
|---|---|---|---|
| SpaceCadetPinball (upstream) | MIT | Código do jogo (submódulo + patches) | Manter aviso de copyright e texto da licença em redistribuições (fonte e binário). Conferir o titular exato no arquivo `LICENSE` do commit fixado `[VERIFICAR]` |
| `libretro.h` | MIT (conforme cabeçalho do arquivo) `[VERIFICAR]` | API do core | Manter cabeçalho |
| Dear ImGui | MIT | **Removido** do build | n/a |
| SDL2 / SDL2_mixer | zlib | **Não linkados**; apenas um shim próprio | n/a |
| TinySoundFont + TinyMidiLoader (opcional) | MIT `[VERIFICAR]` | Música MIDI (Fase 7) | Incluir aviso no `THIRD_PARTY_LICENSES.md` |
| dr_wav (opcional) | Domínio público / MIT-0 `[VERIFICAR]` | Decodificar WAV dos sons | Idem |
| font8x8 (opcional) | Domínio público `[VERIFICAR]` | Tela de erro | Idem |
| Código novo deste projeto | **MIT** (recomendado: mesma do upstream, simples e compatível) | Adaptador libretro, shim, tools | Cabeçalho SPDX em cada arquivo |
| **Assets do jogo** (`PINBALL.DAT`, `CADET.DAT`, `PINBALL.MID`, sprites, sons, música) | **Proprietário (Microsoft / Cinematronics / Maxis)** | **Nunca incluídos** | Fornecidos pelo usuário em runtime |

Observação de risco residual: o upstream é uma decompilação de um binário proprietário, publicada sob MIT pelo autor e
amplamente portada. A licença MIT do upstream **não garante** nada sobre direitos de terceiros sobre o código-fonte
original ou os assets. Ao publicar um derivado, você assume o mesmo risco residual do upstream; documente isso no
`LEGAL.md`.

### 3.2 Regras invioláveis (aplicar em todo o repositório)

1. **Nenhum asset original** no repositório, nos releases, nas imagens Docker, nos caches de CI nem em fixtures de teste.
2. **Nenhum derivado de asset**: nada extraído, convertido, recomprimido, resumido em tabelas ou “embutido” (ex.: paleta
   copiada, trechos de sprite, WAV de teste tirado do DAT, hashes de frame commitados que permitam reconstruir arte).
3. **Nenhuma etapa de build que leia o DAT.** Diferente de abordagens que geram código/ROM a partir do arquivo de dados
   original, aqui o core lê o DAT **somente em runtime**, no aparelho do usuário.
4. **Testes automatizados nunca dependem do DAT.** Testes que precisam dele são locais, opt-in via variável de ambiente
   (`SCP_DAT_PATH`) e documentados como tal.
5. **Screenshots**: não incluir capturas de gameplay no README/releases (mostram arte da Microsoft). Se quiser ilustrar, use
   diagramas próprios ou a tela de erro/ajuda do core.
6. **Sem links para sites de download de assets/abandonware.** O README orienta o usuário a usar a cópia que ele possui
   legitimamente (ex.: instalação própria do Windows XP ou do Full Tilt! Pinball) sem apontar fontes.
7. **Atribuição**: manter créditos ao autor do upstream e contribuidores, e aos criadores do jogo original.
8. **Marca**: não usar logotipos/arte do jogo; nomear o projeto de forma descritiva e com aviso de “não afiliado”.
9. **Política de contribuição**: PRs que contenham assets, dumps, ou código copiado de fontes incompatíveis são rejeitados.

### 3.3 O que o usuário fornece e onde

Layout no cartão SD (nomes de pastas `[VERIFICAR]` contra o bundle “extras” do MinUI):

```
SD/
├─ Emus/miyoomini/PINBALL.pak/        <- distribuído por este projeto
│   ├─ launch.sh
│   ├─ default.cfg
│   ├─ spacecadetpinball_libretro.so
│   └─ README.txt
├─ Roms/Space Cadet Pinball (PINBALL)/ <- criada pelo usuário; a tag entre parênteses liga a pasta ao pak
│   └─ PINBALL.DAT                     <- fornecido pelo usuário (não distribuído)
├─ Bios/PINBALL/                       <- opcional: soundfont (.sf2) para a música
└─ Saves/PINBALL/                      <- criada automaticamente (pontuações etc.)
```

O `PINBALL.MID` (música), se existir no conjunto de arquivos do usuário, também é lido de **ao lado do DAT** em runtime.

### 3.4 Guardas automatizadas

- `.gitignore` com `*.dat`, `*.DAT`, `*.mid`, `*.MID`, `*.sf2`, `*.exe`, `*.bin` e a pasta `local/`.
- `tools/check-no-proprietary.sh` (Apêndice C): falha se encontrar nomes proibidos, arquivos binários grandes ou o
  cabeçalho mágico do DAT em qualquer arquivo versionado. Roda no **pre-commit** e no **CI**.
- **Empacotamento por whitelist**: `tools/package.sh` monta o `.zip` listando explicitamente os arquivos permitidos; nunca
  “zipa a pasta”.
- Revisão de dependências: `THIRD_PARTY_LICENSES.md` gerado/atualizado a cada release e conferido no CI.
- O submódulo do upstream traz `Screenshots/` e outros arquivos; como **não** os redistribuímos (só referenciamos o commit),
  isso é aceitável, mas o pacote de release nunca inclui nada do submódulo além do que o compilador gerou.

### 3.5 Créditos e avisos (README + `NOTICE.md`)

- Andrey Muzychenko (k4zmu2a) e contribuidores — decompilação SpaceCadetPinball.
- Cinematronics (jogo original), Maxis (publicação do Full Tilt! Pinball), Microsoft (edição Windows).
- Aviso: “Projeto não oficial, não afiliado nem endossado por Microsoft, Maxis, Cinematronics ou Shaun Inman. Nenhum dado
  do jogo é distribuído. Você precisa fornecer seus próprios arquivos obtidos legitimamente.”

---

## 4. Arquitetura

### 4.1 Camadas

```
┌─────────────────────────────── MinUI / minarch.elf ───────────────────────────────┐
│   carrega o .so, chama retro_* , desenha o framebuffer, mixa áudio, lê botões     │
└───────────────────────────────────────▲───────────────────────────────────────────┘
                                        │ libretro API (C, símbolos retro_* exportados)
┌───────────────────────────────────────┴───────────────────────────────────────────┐
│  src/libretro/   (código NOVO, MIT)                                               │
│   libretro.cpp   ─ ponto de entrada, ciclo de vida, validação de conteúdo         │
│   video.cpp      ─ 8bpp indexado → RGB565, mensagens de erro                      │
│   audio.cpp      ─ mixer de SFX (e música opcional) → int16 estéreo               │
│   input.cpp      ─ RetroPad → eventos do jogo                                     │
│   options.cpp    ─ core options, high scores, persistência                        │
│   state.cpp      ─ serialização (fase posterior)                                  │
│   clock.cpp      ─ relógio determinístico (frames → ms)                           │
├───────────────────────────────────────────────────────────────────────────────────┤
│  shim/   (headers/impls mínimos no lugar de SDL2, SDL_mixer, ImGui)               │
├───────────────────────────────────────────────────────────────────────────────────┤
│  third_party/SpaceCadetPinball/   (submódulo upstream, commit fixado, MIT)        │
│  patches/*.patch                  (mudanças mínimas e documentadas)               │
└───────────────────────────────────────────────────────────────────────────────────┘
```

### 4.2 Estratégia de integração com o upstream `[DECISÃO]`

Três opções:

| Opção | Prós | Contras |
|---|---|---|
| **A. Submódulo + shim de SDL + poucos patches** (recomendada) | Mantém o upstream intocado; fácil rebasear; atribuição clara; diffs pequenos | Exige escrever o shim; alguns patches inevitáveis |
| B. Fork vendorizado (copiar fontes para o repo) | Liberdade total | Divergência rápida; mais difícil de acompanhar upstream |
| C. Reescrever partes do zero | Código limpo | Perde fidelidade; maior risco de bugs de física/regras |

**Recomendação:** A, com um *decision gate* na Fase 0: se o inventário de chamadas SDL mostrar uso espalhado demais para um
shim viável, migrar para B (fork) mantendo os mesmos adaptadores.

Regras para patches: um assunto por patch, arquivos em `patches/NNNN-descricao.patch`, aplicados no configure (`git apply`),
cada um com cabeçalho explicando o porquê e se é candidato a upstream. Todo patch aplicável ao desktop deve preservar o
comportamento original.

### 4.3 Pontos de contato prováveis a substituir `[VERIFICAR na Fase 0]`

O inventário real vem de `grep -rn "SDL_\|Mix_\|ImGui\|imgui" third_party/SpaceCadetPinball/SpaceCadetPinball`. Espera-se:

| Área | Upstream (provável) | Substituição no core |
|---|---|---|
| Loop principal, eventos, janela | `winmain.cpp` (SDL window/renderer/eventos, loop `while`) | `retro_run()` executa **um passo**; sem janela |
| Renderização | `gdrv.cpp`/`render.cpp` escrevem num bitmap 8bpp indexado + paleta; `winmain` copia para textura SDL | Ler o mesmo bitmap + paleta e converter para RGB565 em `video.cpp` |
| UI / menus | Dear ImGui (opções, high scores, about) | Removida; opções via core options; high scores via SAVE_RAM |
| Som (SFX) | `Sound.cpp` + SDL_mixer (`Mix_LoadWAV_RW`, canais, volume) | Mixer próprio (`audio.cpp`) sobre as amostras do DAT |
| Música | `midi.cpp` + SDL_mixer (MIDI) | Opcional: TinySoundFont/TML com `.sf2` do usuário; MVP sem música |
| Tempo | `SDL_GetTicks` / contadores de alta resolução | `clock.cpp`: tempo avança em passos fixos por frame |
| Arquivos | `SDL_GetBasePath`/`SDL_GetPrefPath`, `fopen` | Caminho vindo de `retro_game_info.path`; diretórios via `GET_SYSTEM_DIRECTORY`/`GET_SAVE_DIRECTORY` |
| Aleatoriedade | `rand()`/`srand(time)` | PRNG com seed controlada (necessário para states) |
| Opções persistentes | arquivo/registro via `options.cpp` | Buffer em memória + core options + SAVE_RAM |
| Log | `printf`/`fprintf` | `retro_log_printf_t` |

Referências úteis (leia como exemplo, respeitando as licenças deles): ports de Wii/3DS (MaikelChan) e DS/PS2/Dreamcast
(Headshotnoby), listados no README do upstream — mostram como cortar SDL/ImGui e lidar com memória/tempo limitados.

### 4.4 Mapeamento da API libretro

| Função libretro | Implementação |
|---|---|
| `retro_set_environment` | Guarda callback; `SET_SUPPORT_NO_GAME=false`; registra variáveis de core options; registra descritores de entrada |
| `retro_get_system_info` | `library_name="SpaceCadetPinball"`, `need_fullpath=true`, `valid_extensions="dat"`, `block_extract=true` |
| `retro_get_system_av_info` | 640×480, aspecto 4:3, `fps=60.0`, `sample_rate=44100.0` `[VERIFICAR taxa efetiva do upstream]` |
| `retro_load_game` | `SET_PIXEL_FORMAT=RGB565`; valida o DAT (magic + tamanho); inicializa o jogo; em falha, entra em “modo erro” e/ou retorna `false` com log claro |
| `retro_run` | Poll de input → um passo de jogo com `dt` fixo → conversão de vídeo → `video_cb` → flush de áudio → `audio_batch_cb` |
| `retro_reset` | Novo jogo (equivalente ao “New Game” original) |
| `retro_unload_game` / `retro_deinit` | Libera tudo; sem estado global residual |
| `retro_get_memory_data/size` | `RETRO_MEMORY_SAVE_RAM` = tabela de high scores (tamanho fixo) |
| `retro_serialize*` / `retro_unserialize` | Fase 8; antes disso `retro_serialize_size()` retorna 0 |
| `retro_cheat_*`, `retro_get_region` | Stubs; região NTSC |

### 4.5 Decisões de projeto

**Vídeo.** O upstream mantém um buffer 8bpp indexado de 640×480 e uma paleta de 256 entradas. Em cada frame: construir/atualizar
uma LUT `uint16_t pal565[256]` quando a paleta mudar e converter linha a linha para o framebuffer RGB565 do core. Versão 1:
converter a tela inteira (≈307 k pixels; barato no A7). Otimização posterior: converter apenas retângulos sujos, se o upstream
já os rastreia. Resolução 1:1 com a tela do aparelho, então o scaler do `minarch` não precisa interpolar.

**Áudio.** Mixer de passo fixo: a cada frame produzir `sample_rate/fps` amostras estéreo (`int16`) somando os canais ativos
(amostras PCM do DAT, volume, pan se existir). Sem threads. Se o formato dos WAV for algo além de PCM, usar `dr_wav` ou
um decodificador mínimo `[VERIFICAR formato real]`. Respeitar sempre o volume global do MinUI (o frontend aplica; o core não
deve embutir ganho extra além do necessário para normalização).

**Entrada.** Proposta de mapeamento para o Miyoo Mini (ajustar em playtest):

| Ação do jogo | RetroPad | Botão físico |
|---|---|---|
| Flipper esquerdo | `L` e `LEFT` | L / ← |
| Flipper direito | `R` e `RIGHT` | R / → |
| Plunger (segurar = puxar, soltar = lançar) | `DOWN` e `A` | ↓ / A |
| Nudge esquerdo | `Y` | Y |
| Nudge direito | `B` | B |
| Nudge de baixo | `UP` | ↑ |
| Novo jogo / lançar bola extra | `START` | Start |
| Pausar | `SELECT` | Select |

Registrar `SET_INPUT_DESCRIPTORS` com rótulos legíveis (“Left Flipper”, “Right Flipper”, “Plunger”, …). O `default.cfg`
referencia esses rótulos.

**Tempo.** Substituir o relógio real por um relógio **determinístico**: cada `retro_run` avança `1000/fps` ms (ou um
inteiro de microssegundos). Se o upstream usa um passo de física diferente do passo de renderização (atualizações por
segundo vs. FPS), preservar essa relação exatamente `[VERIFICAR]`. Isso é pré-requisito para save states e facilita testes
reprodutíveis.

**Opções do core (MVP):** `scp_music` (on/off), `scp_plunger_mode` (hold-release / tap), `scp_nudge` (on/off),
`scp_show_fps` (debug). Usar `SET_VARIABLES` legado por compatibilidade máxima com o `minarch` `[VERIFICAR qual versão de
core options o minarch implementa em minarch.c]`.

**Persistência.** High scores em `SAVE_RAM` (tamanho fixo, formato versionado, little-endian). Opções pelo mecanismo de core
options do frontend; nada de arquivos `.ini` próprios.

**Mensagens ao usuário.** Se o DAT não for encontrado/for inválido, renderizar uma tela própria (fonte bitmap embutida)
dizendo o que fazer, além de logar. Evita “tela preta” sem explicação em um aparelho sem teclado/console.

**Memória.** Alocar o mínimo necessário; preferir buffers estáticos dimensionados após medir. Ver orçamento na seção 11.

---

## 5. Estrutura do repositório

```
spacecadetpinball-libretro/
├─ .github/
│   ├─ workflows/ci.yml
│   ├─ workflows/release.yml
│   └─ PULL_REQUEST_TEMPLATE.md      # checklist: "não incluí assets"
├─ .cursor/rules/scp-libretro.mdc    # regras para o Cursor (Apêndice E)
├─ cmake/
│   ├─ miyoomini.toolchain.cmake
│   └─ patches.cmake                 # aplica patches/*.patch de forma idempotente
├─ docker/
│   └─ Dockerfile.dev                # opcional: ambiente de dev host (clang-format, etc.)
├─ docs/
│   ├─ ARCHITECTURE.md
│   ├─ BUILDING.md
│   ├─ USER_GUIDE.md                 # como instalar no MinUI e onde pôr o DAT
│   ├─ LEGAL.md
│   ├─ INPUT_MAPPING.md
│   └─ STATE_SERIALIZATION.md        # Fase 8
├─ include/shim/                     # headers falsos: SDL.h, SDL_mixer.h, imgui stubs (se necessário)
├─ pak/PINBALL.pak/
│   ├─ launch.sh
│   ├─ default.cfg
│   └─ README.txt
├─ patches/                          # NNNN-*.patch contra o upstream
├─ src/libretro/
│   ├─ libretro.cpp
│   ├─ video.cpp  video.h
│   ├─ audio.cpp  audio.h
│   ├─ input.cpp  input.h
│   ├─ options.cpp options.h
│   ├─ clock.cpp  clock.h
│   ├─ state.cpp  state.h
│   ├─ errorscreen.cpp errorscreen.h
│   └─ log.h
├─ third_party/
│   ├─ SpaceCadetPinball/            # submódulo (commit/tag fixados)
│   └─ libretro/libretro.h           # cópia do cabeçalho (MIT) com versão anotada
├─ tests/
│   ├─ unit/                         # sem DAT
│   ├─ smoke/                        # carrega o .so, valida símbolos e caminho de erro (sem DAT)
│   └─ local/                        # exige SCP_DAT_PATH; NUNCA roda no CI
├─ tools/
│   ├─ check-no-proprietary.sh
│   ├─ sdl-inventory.sh
│   ├─ package.sh                    # whitelist
│   ├─ headless-runner/              # carrega o core, injeta inputs, dumpa frames/áudio (local)
│   └─ gen-third-party-licenses.sh
├─ CMakeLists.txt
├─ CHANGELOG.md
├─ CONTRIBUTING.md
├─ LICENSE                           # MIT (código novo)
├─ NOTICE.md                         # créditos e avisos
├─ THIRD_PARTY_LICENSES.md
└─ README.md
```

---

## 6. Sistema de build

### 6.1 Princípios

- **CMake** como sistema principal (o upstream já usa CMake). Duas configurações: `host` (desenvolvimento, ASan/UBSan) e
  `miyoomini` (cross-compile na toolchain Docker). Se um dia for upstream no libretro, adicionar um `Makefile.libretro`.
- **O build nunca lê o DAT** e nunca baixa assets. Falha de build não pode depender de arquivos proprietários.
- Reprodutível: versão da toolchain Docker fixada (por digest/commit), submódulo fixado, `SOURCE_DATE_EPOCH` definido.

### 6.2 Build de host (desenvolvimento)

```bash
git clone --recurse-submodules https://github.com/<voce>/spacecadetpinball-libretro.git
cd spacecadetpinball-libretro
cmake -S . -B build/host -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSCP_SANITIZE=ON
cmake --build build/host -j
# Rodar no RetroArch de desktop (usando o SEU DAT, fora do repositório):
retroarch -L build/host/spacecadetpinball_libretro.so "$HOME/games/pinball/PINBALL.DAT"
```

### 6.3 Build de cross-compile para Miyoo Mini

```bash
# 1) Subir a toolchain (fixe o commit/digest que você testou)
git clone https://github.com/shauninman/union-miyoomini-toolchain.git
cd union-miyoomini-toolchain && make shell      # monta ./workspace em ~/workspace no container
# 2) Dentro do container (o projeto está em ~/workspace/spacecadetpinball-libretro):
cmake -S . -B build/miyoomini \
  -DCMAKE_TOOLCHAIN_FILE=cmake/miyoomini.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/miyoomini -j
./tools/package.sh build/miyoomini   # gera dist/PINBALL.pak e dist/*.zip
```

Notas:

- Se o `make shell` falhar por repositórios Debian arquivados, ver o patch do Dockerfile / imagem pré-construída do LessUI
  citada na seção 2 (item 8) `[VERIFICAR]`.
- Copiar **as flags de arquitetura exatas** do MinUI (`workspace/miyoomini/platform/makefile.env` ou equivalente) em vez de
  inventar `[VERIFICAR]`. Ponto de partida seguro: `-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O2`.
- **Linkagem do runtime C++:** usar `-static-libstdc++ -static-libgcc` para não depender de `libstdc++.so` no aparelho
  `[VERIFICAR o que existe no firmware]`. Ocultar símbolos (`-fvisibility=hidden`) e exportar só `retro_*` via version script
  (Apêndice D) para evitar colisões com símbolos do `minarch`.
- **Checagens pós-build (no CI):** `readelf -d` (só `libc`, `libm`, `libdl`, `libpthread` em `NEEDED`), `objdump -T | grep GLIBC_`
  (versão máxima de símbolos ≤ a do aparelho), `file` (ARM, EABI5, hard-float), tamanho do `.so`.

### 6.4 Opções de build

| Opção CMake | Padrão | Efeito |
|---|---|---|
| `SCP_ENABLE_MUSIC` | `OFF` no MVP | Compila TinySoundFont/TML |
| `SCP_SANITIZE` | `OFF` | ASan/UBSan (somente host) |
| `SCP_LOG_LEVEL` | `INFO` | Verbosidade do log |
| `SCP_DETERMINISTIC_RNG` | `ON` | PRNG com seed controlada |
| `SCP_BUILD_TESTS` | `ON` no host | Testes unitários/smoke |

---

## 7. Empacotamento do pak

### 7.1 `launch.sh`

Baseado no exemplo do `PAKS.md` (core incluído no próprio pak, tipo 2):

```sh
#!/bin/sh

EMU_EXE=spacecadetpinball
CORES_PATH=$(dirname "$0")

###############################

EMU_TAG=$(basename "$(dirname "$0")" .pak)
ROM="$1"
mkdir -p "$BIOS_PATH/$EMU_TAG"
mkdir -p "$SAVES_PATH/$EMU_TAG"
HOME="$USERDATA_PATH"
cd "$HOME"
minarch.elf "$CORES_PATH/${EMU_EXE}_libretro.so" "$ROM" &> "$LOGS_PATH/$EMU_TAG.txt"
```

### 7.2 `default.cfg`

Gerar do jeito descrito no `PAKS.md`: abrir o jogo no aparelho, configurar em Menu → Options, “Save Changes → Save for Console”,
copiar o `minarch.cfg` resultante de `.userdata/<plataforma>/PINBALL-spacecadetpinball/` e reduzir às opções que você quer
fixar. Esqueleto (os rótulos **devem coincidir** com os descritores de entrada do core `[VERIFICAR em minarch.c]`):

```
bind Left Flipper = L1
bind Right Flipper = R1
bind Plunger = A
bind Nudge Left = Y
bind Nudge Right = B
bind Nudge Up = UP
bind New Game = START
bind Pause = SELECT
```

### 7.3 `README.txt` dentro do pak

Instruções curtas para o usuário final (em EN e PT-BR): onde criar a pasta de ROMs, como nomeá-la com a tag `(PINBALL)`,
qual arquivo copiar, o que fazer se aparecer a tela de erro, onde está o log. **Sem links para fontes de assets.**

---

## 8. Plano de implementação por fases

Legenda: ☐ tarefa · **Aceite** = critério objetivo para considerar a fase concluída.

### Fase 0 — Preparação, spike e decision gates (1–2 dias)

**Objetivo:** reduzir incertezas antes de escrever código de produção.

- ☐ Criar o repositório, `LICENSE` (MIT), `NOTICE.md`, `.gitignore`, `check-no-proprietary.sh` e o hook de pre-commit **antes** do primeiro commit de código.
- ☐ Adicionar o upstream como submódulo e fixar um **tag/commit** (`git describe --tags`); registrar no `docs/ARCHITECTURE.md`.
- ☐ Repetir a busca por um core libretro existente do jogo; registrar o resultado.
- ☐ Compilar o upstream no host (Linux) e rodar com o seu DAT. Registrar baseline: FPS, uso de CPU, RSS de memória.
- ☐ Rodar `tools/sdl-inventory.sh` (grep de `SDL_`, `Mix_`, `ImGui`) e produzir uma tabela de chamadas por arquivo.
- ☐ Ler: `winmain.cpp` (loop), `gdrv`/`render` (bitmap + paleta), `Sound.cpp`/`midi.cpp`, `options.cpp`, `partman.cpp` (formato/magic do DAT), `score`/high scores, geração de números aleatórios.
- ☐ Ler `minarch.c` do MinUI (`workspace/all/minarch/`): quais chamadas de ambiente são tratadas, versão de core options, rótulos de botões, diretórios (system/save) `[VERIFICAR]`.
- ☐ Subir a toolchain Docker e compilar um “hello libretro core” (retro_* vazios) para `miyoomini`; carregar no aparelho via pak mínimo (tela cinza serve).
- ☐ **Gate 1:** shim viável (Opção A) ou fork (Opção B)?
- ☐ **Gate 2:** a lógica do jogo roda sem nenhuma chamada SDL residual no caminho de `retro_run`?

**Aceite:** relatório `docs/PHASE0_FINDINGS.md` (sem nenhuma informação derivada de assets), core vazio carregando no MinUI, decisões registradas.

### Fase 1 — Esqueleto do core e pipeline de build (2–3 dias)

- ☐ Implementar todas as funções `retro_*` (stubs seguros), `retro_get_system_info/av_info`, log via callback.
- ☐ `CMakeLists.txt` com targets host e `miyoomini`; toolchain file; version script exportando só `retro_*`.
- ☐ `tools/package.sh` (whitelist) e geração do `PINBALL.pak`.
- ☐ Tela de erro (`errorscreen.cpp`): fundo sólido + texto bitmap “PINBALL.DAT not found…”.
- ☐ Validação de conteúdo: existência, tamanho mínimo, magic header; mensagens específicas para cada falha.
- ☐ CI inicial: lint + `check-no-proprietary` + build host + build `miyoomini` + checagens de ELF.
- ☐ Teste smoke (`tests/smoke`): `dlopen` do `.so`, todos os símbolos `retro_*` presentes, `retro_load_game` com arquivo inexistente falha de forma limpa, sem crash.

**Aceite:** pak instalado no aparelho mostra a tela de erro quando falta o DAT; CI verde; nenhum arquivo proibido versionado.

### Fase 2 — Núcleo do jogo sem SDL (3–5 dias)

- ☐ Criar o shim mínimo (`include/shim/`) e/ou patches para compilar os fontes do upstream **sem** SDL2/SDL_mixer/ImGui linkados.
- ☐ Substituir o loop principal por `scp::game::init()`, `scp::game::step(dt)`, `scp::game::shutdown()`.
- ☐ Remover/stubar tudo de UI (ImGui), janela, eventos, fullscreen.
- ☐ Caminhos de arquivo: derivar a pasta do DAT a partir de `retro_game_info.path`.
- ☐ Logging via libretro; remover `printf` solto.
- ☐ Relógio determinístico (`clock.cpp`) e PRNG com seed controlada.
- ☐ Carregar o DAT e inicializar a mesa **no host** com o seu DAT; verificar ausência de vazamentos e de acessos inválidos (ASan).

**Aceite (local, com DAT):** no host, `step()` roda 10 minutos simulados sem erro de sanitizers; o estado do jogo avança (bola em jogo, pontuação muda) sob inputs injetados.

### Fase 3 — Vídeo (3–5 dias)

- ☐ Expor o bitmap 8bpp indexado e a paleta do upstream ao adaptador (ponteiros + dimensões + “paleta mudou”).
- ☐ `video.cpp`: LUT de paleta → RGB565; conversão linha a linha com tratamento de *stride*; `video_cb` 640×480 com `pitch = 640*2`.
- ☐ Garantir que a 1ª frame válida só é entregue após inicialização completa (sem lixo).
- ☐ Medir custo da conversão no aparelho; se > ~2 ms, otimizar (NEON, retângulos sujos).
- ☐ Overlay de depuração opcional (FPS) controlado por core option.

**Aceite (local):** mesa renderiza corretamente no RetroArch desktop e no aparelho; sem tearing/artefatos; paleta correta (inclusive animações de luzes/cores).

### Fase 4 — Entrada (2–3 dias)

- ☐ `SET_INPUT_DESCRIPTORS` e leitura via `input_state_cb`.
- ☐ Traduzir RetroPad → eventos/estado do jogo (flippers, plunger com segurar/soltar, nudges, novo jogo, pausa).
- ☐ Tratar bordas (pressionou/soltou) com estado do frame anterior; sem *key repeat* espúrio.
- ☐ Opção `scp_plunger_mode`.
- ☐ `default.cfg` com rótulos coincidentes e testado no MinUI.

**Aceite:** é possível jogar uma partida completa no aparelho: lançar, jogar com os flippers, nudge, game over, novo jogo.

### Fase 5 — Áudio de efeitos (2–3 dias)

- ☐ Extrair do DAT (em runtime) as amostras já carregadas pelo upstream e entregá-las ao `audio.cpp`.
- ☐ Mixer: N canais, soma saturada em `int16`, `sample_rate/fps` amostras por frame (acumular o resto quando não for inteiro).
- ☐ Honrar eventos “tocar/parar/volume” da camada `Sound` do upstream.
- ☐ Zero alocação no caminho quente; sem clipping audível (limiter simples).
- ☐ Teste local: gravar 30 s de áudio num WAV de depuração (**fora do repo**) e inspecionar.

**Aceite:** efeitos corretos e sincronizados, sem *underruns*/estalos no aparelho por 10 min seguidos.

### Fase 6 — Persistência, opções e MVP (1–2 dias)

- ☐ High scores em `SAVE_RAM` (formato versionado, validação de tamanho/CRC).
- ☐ Core options finalizadas e legíveis no menu do MinUI.
- ☐ `retro_reset` = novo jogo.
- ☐ Revisar mensagens de erro e logs.
- ☐ Primeiro release interno `v0.1.0-mvp`.

**Aceite:** critérios do MVP (seção 1.3) atendidos no aparelho real; high scores persistem entre sessões.

### Fase 7 — Música MIDI opcional (2–3 dias)

- ☐ Integrar TinySoundFont + TinyMidiLoader atrás de `SCP_ENABLE_MUSIC`.
- ☐ Soundfont `.sf2` **fornecido pelo usuário** em `Bios/PINBALL/` (não distribuir); documentar requisitos e licença do soundfont escolhido pelo usuário.
- ☐ Renderização de música por frame com limite de vozes para caber no orçamento de CPU; fallback silencioso se faltar `.sf2` ou `.MID`.
- ☐ Mixagem com SFX; core option para ligar/desligar.

**Aceite:** música toca sem derrubar o frame time acima do orçamento; sem erro quando os arquivos opcionais não existem.

### Fase 8 — Save states / auto-resume (5–10 dias; alto risco)

**Por que é difícil:** o estado do jogo está espalhado por muitos objetos (componentes da mesa, bola, flippers, timers, máquina de estados das missões, pontuação, PRNG). O `minarch` usa states para *quicksave* e para o auto-resume ao religar.

- ☐ **Spike (1–2 dias) — decision gate `[DECISÃO]`:**
  - **Opção S1 — Serialização explícita:** enumerar e serializar campos de cada classe relevante; versão de formato; tamanho máximo fixo. Mais trabalho, mais robusta, tamanho pequeno.
  - **Opção S2 — Snapshot de arena:** alocar todo o estado dinâmico num arena de endereço fixo (ex.: `mmap` com endereço fixo) e copiar também os globais relevantes. Rápida de implementar, mas frágil (ponteiros para fora do arena, vtables/funções, `std::` containers, futuras atualizações do upstream) e pode produzir states grandes.
  - **Fallback:** manter `retro_serialize_size() == 0` e documentar que quicksave/auto-resume não existem.
- ☐ Implementar a opção escolhida em `state.cpp`; `STATE_VERSION` + hash do build no cabeçalho do state.
- ☐ Teste de determinismo (local): `rodar N frames → serializar → rodar M frames → hash(vídeo+áudio)`; `restaurar → rodar M frames → hash` deve ser idêntico, com os mesmos inputs.
- ☐ Teste de robustez: state corrompido/de versão diferente é rejeitado sem crash.

**Aceite:** quicksave/quickload e auto-resume funcionam no aparelho; teste de determinismo passa 100% em 20 execuções com sequências de input diferentes.

### Fase 9 — Polimento, documentação e release (2–3 dias)

- ☐ Perfil de desempenho no aparelho e ajustes finais (seção 11).
- ☐ `README.md`, `USER_GUIDE.md`, `BUILDING.md`, `LEGAL.md`, `INPUT_MAPPING.md`, `ARCHITECTURE.md`, `CONTRIBUTING.md`, `CHANGELOG.md`.
- ☐ `THIRD_PARTY_LICENSES.md` revisado; cabeçalhos SPDX em todos os arquivos novos.
- ☐ Workflow de release: tag `vX.Y.Z` → build `miyoomini` → whitelist → `.zip` + `SHA256SUMS` + notas.
- ☐ Revisão final de LEGAL: checklist da seção 3 + varredura manual do repo e do artefato de release (nenhum arquivo do jogo).
- ☐ (Opcional) Abrir issue/PR upstream com patches genéricos úteis ao projeto original; avaliar upstreaming no ecossistema libretro (verificar a política de contribuição deles para jogos baseados em engenharia reversa).

**Aceite:** release `v1.0.0` publicada; um usuário novo consegue seguir o `USER_GUIDE.md` do zero e jogar.

### Visão geral de esforço

| Fase | Estimativa |
|---|---|
| 0 Preparação/spikes | 1–2 d |
| 1 Esqueleto + build | 2–3 d |
| 2 Núcleo sem SDL | 3–5 d |
| 3 Vídeo | 3–5 d |
| 4 Entrada | 2–3 d |
| 5 Áudio SFX | 2–3 d |
| 6 Persistência + MVP | 1–2 d |
| **Subtotal MVP** | **≈ 14–23 d** |
| 7 Música | 2–3 d |
| 8 Save states | 5–10 d |
| 9 Polimento + release | 2–3 d |
| **Total v1.0+states** | **≈ 23–39 d** |

---

## 9. Estratégia de testes

| Camada | O que testar | Depende do DAT? | Onde roda |
|---|---|---|---|
| Unitários | Conversão 8bpp→RGB565, LUT de paleta, mixer (saturação, resto fracionário), parser de high scores, relógio determinístico, PRNG, validação de cabeçalho com buffers sintéticos | **Não** | CI + local |
| Smoke | `dlopen` do `.so`, símbolos `retro_*`, caminho de erro sem DAT, ausência de crash em load/unload repetidos | **Não** | CI + local |
| Integração local | Carregar o DAT real, rodar *scripts* de input, validar que a pontuação muda, que o áudio é gerado, ausência de erros de sanitizers | **Sim** (`SCP_DAT_PATH`) | Somente local |
| Determinismo | `serialize/unserialize` (Fase 8) | **Sim** | Somente local |
| Manual no aparelho | Playtest, bateria, sleep/wake, troca de volume/brilho, uso prolongado | **Sim** | Aparelho |

Regras:

- Fixtures de teste são **sintéticas** (geradas por código). Nunca copiar bytes do DAT para fixtures.
- Dumps de vídeo/áudio gerados localmente vão para `local/` (ignorada pelo git).
- O `headless-runner` carrega o `.so`, injeta inputs de um arquivo de script e grava hashes; é para uso local.

---

## 10. CI/CD (GitHub Actions)

`ci.yml` (push/PR):

1. **lint** — `clang-format --dry-run`, `shellcheck`, verificação de cabeçalhos SPDX, `check-no-proprietary.sh`.
2. **host-build-test** — Ubuntu LTS, gcc e clang, `SCP_SANITIZE=ON`, roda unitários + smoke.
3. **miyoomini-build** — container da toolchain (imagem fixada por digest), build Release, checagens de ELF (`NEEDED`, `GLIBC_`, arquitetura), upload do `PINBALL.pak` como artefato de CI.
4. **license-check** — confere que `THIRD_PARTY_LICENSES.md` está em dia e que o submódulo está no commit fixado.

`release.yml` (tag `v*`): reaproveita o job 3, executa `tools/package.sh` (whitelist), gera `SHA256SUMS`, cria a release no GitHub com notas do `CHANGELOG.md`. O workflow **falha** se o `.zip` contiver qualquer arquivo fora da lista permitida.

Boas práticas: *pin* de versões de actions por SHA, cache de build sem conter assets, permissões mínimas do `GITHUB_TOKEN`.

---

## 11. Orçamento de desempenho e memória

Metas iniciais (validar na Fase 0/3 e ajustar):

| Métrica | Meta |
|---|---|
| Frame time médio (física + render + conversão + áudio) | ≤ 10 ms |
| Frame time p99 | ≤ 16 ms (60 fps estáveis) |
| Conversão 8bpp→RGB565 | ≤ 1,5 ms |
| Mixer de SFX | ≤ 0,5 ms por frame |
| Música MIDI (opcional) | ≤ 3 ms por frame |
| Memória residente do core | bem abaixo do que sobra no aparelho; medir e fixar teto (ex.: ≤ 40 MB) `[VERIFICAR]` |
| Tamanho do `.so` | ≤ ~2 MB (sem ImGui/SDL) |

Como medir: `clock_gettime(CLOCK_MONOTONIC)` em volta de cada etapa dentro do core (ativável por `scp_show_fps`/log),
agregando médias e p99 a cada N frames; no host, `perf`/`valgrind --tool=callgrind` para hot spots. Considerar o modo de
CPU do MinUI no aparelho durante os testes e registrar em qual configuração a meta foi atingida.

---

## 12. Registro de riscos

| # | Risco | Prob. | Impacto | Mitigação |
|---|---|---|---|---|
| R1 | Uso de SDL no upstream mais espalhado do que o esperado | M | M | Gate 1 na Fase 0; fallback para fork (Opção B) |
| R2 | Física dependente de tempo real / `double` de forma que quebra com passo fixo | M | A | Relógio determinístico desde a Fase 2; comparar comportamento com o desktop; testes de regressão locais |
| R3 | Desempenho insuficiente no A7 | B/M | A | Medir cedo (Fase 0/3); NEON, retângulos sujos, reduzir uso de `double` onde seguro |
| R4 | Formato dos sons no DAT exige decodificador extra | B | B | `dr_wav`/decodificador mínimo; verificar na Fase 0 |
| R5 | Save states inviáveis ou frágeis | A | M | Spike da Fase 8; fallback sem states, documentado |
| R6 | Incompatibilidade de ABI/glibc/libstdc++ com o firmware | M | A | Link estático do runtime C++; checagens de ELF no CI; teste no aparelho desde a Fase 0 |
| R7 | `minarch` não implementa alguma chamada de ambiente usada | M | B | Ler `minarch.c` na Fase 0; usar apenas o subconjunto suportado; degradar com elegância |
| R8 | Toolchain Docker quebra (repos Debian arquivados) | M | M | Fixar imagem por digest; documentar o workaround; manter cópia do Dockerfile corrigido (se a licença permitir) |
| R9 | Arquivo proprietário commitado por engano | B | **Crítico** | `check-no-proprietary` em pre-commit + CI + whitelist de release + PR template |
| R10 | Reclamação de titular de direitos sobre o projeto | B | A | `LEGAL.md`, sem assets, sem marcas/arte, resposta rápida a pedidos de remoção; considerar revisão jurídica |
| R11 | MinUI arquivado: mudanças futuras/forks incompatíveis | M | B | Documentar versão testada; testar também em forks se desejado |
| R12 | Divergência do upstream ao longo do tempo | M | B | Submódulo fixado + patches pequenos; atualizar por tag, em PRs separados |

---

## 13. Decisões em aberto

1. **Integração com o upstream:** A (submódulo+shim) vs. B (fork) — decidir no Gate 1.
2. **Save states:** S1 (explícita) vs. S2 (arena) vs. sem states — decidir no spike da Fase 8.
3. **Música:** incluir no v1.0 ou só depois? Exige `.sf2` do usuário.
4. **Nome do core/pak e da tag:** sugestão: core `spacecadetpinball`, tag `PINBALL`; checar colisões com cores existentes.
5. **Plataformas adicionais** (ex.: outros aparelhos suportados pelo MinUI): fora do escopo até o v1.0.
6. **Licença do código novo:** MIT (recomendada) — confirmar.
7. **Suporte ao DAT do Full Tilt vs. só ao do Windows:** o upstream diz suportar ambos; testar ambos se possível.

---

## 14. Entregáveis de documentação

- `README.md` (EN): o que é, aviso legal em destaque, como instalar, como compilar, limitações.
- `docs/USER_GUIDE.md` (EN + PT-BR): passo a passo de instalação no MinUI, onde colocar o DAT, solução de problemas, localização do log.
- `docs/BUILDING.md`: toolchain, comandos, flags, checagens de ELF.
- `docs/ARCHITECTURE.md`: camadas, fluxo de frame, decisões e *trade-offs*.
- `docs/LEGAL.md`: política de assets, licenças, créditos, procedimento para pedidos de remoção.
- `docs/INPUT_MAPPING.md`, `docs/STATE_SERIALIZATION.md`.
- `CONTRIBUTING.md`: estilo de código, checklist “sem assets”, como rodar os testes locais.

---

## Apêndice A — Esqueleto de `libretro.cpp`

```cpp
// SPDX-License-Identifier: MIT
#include "libretro.h"
#include <cstring>
#include <cstdio>

namespace {
retro_environment_t        env_cb;
retro_video_refresh_t      video_cb;
retro_audio_sample_t       audio_cb;
retro_audio_sample_batch_t audio_batch_cb;
retro_input_poll_t         input_poll_cb;
retro_input_state_t        input_state_cb;
retro_log_printf_t         log_cb;
bool                       game_ready = false;
}  // namespace

extern "C" {

void retro_set_environment(retro_environment_t cb) {
  env_cb = cb;
  bool no_game = false;
  env_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_game);

  struct retro_log_callback log;
  if (env_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log)) log_cb = log.log;

  // TODO: SET_VARIABLES (core options) e SET_INPUT_DESCRIPTORS.
}

void retro_set_video_refresh(retro_video_refresh_t cb)          { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb)            { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb){ audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb)                { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb)              { input_state_cb = cb; }

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_get_system_info(struct retro_system_info *info) {
  std::memset(info, 0, sizeof(*info));
  info->library_name     = "SpaceCadetPinball";
  info->library_version  = "0.1.0";
  info->need_fullpath    = true;     // lemos o DAT (e arquivos vizinhos) pelo caminho
  info->valid_extensions = "dat";
  info->block_extract    = true;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  std::memset(info, 0, sizeof(*info));
  info->geometry.base_width   = 640;
  info->geometry.base_height  = 480;
  info->geometry.max_width    = 640;
  info->geometry.max_height   = 480;
  info->geometry.aspect_ratio = 4.0f / 3.0f;
  info->timing.fps            = 60.0;     // [VERIFICAR] relação com o passo de física do upstream
  info->timing.sample_rate    = 44100.0;
}

bool retro_load_game(const struct retro_game_info *game) {
  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
  if (!env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) return false;
  if (!game || !game->path) return false;

  // 1) validar arquivo (existência, tamanho, magic)  -> modo erro se falhar
  // 2) scp::game::init(game->path, system_dir, save_dir)
  game_ready = false;  // true após init OK
  return true;         // retornar true com "modo erro" mostra a tela de ajuda em vez de falhar mudo
}

void retro_run(void) {
  input_poll_cb();
  // scp::input::poll(input_state_cb);
  // if (game_ready) scp::game::step();          // dt fixo
  // scp::video::present(video_cb, game_ready);  // 8bpp -> RGB565 ou tela de erro
  // scp::audio::flush(audio_batch_cb);          // N amostras estéreo por frame
}

void   retro_reset(void)                       { /* novo jogo */ }
void   retro_unload_game(void)                 { /* scp::game::shutdown() */ game_ready = false; }
void   retro_init(void)                        {}
void   retro_deinit(void)                      {}
void   retro_set_controller_port_device(unsigned, unsigned) {}
unsigned retro_get_region(void)                { return RETRO_REGION_NTSC; }
bool   retro_load_game_special(unsigned, const struct retro_game_info *, size_t) { return false; }

size_t retro_serialize_size(void)              { return 0; }  // Fase 8
bool   retro_serialize(void *, size_t)         { return false; }
bool   retro_unserialize(const void *, size_t) { return false; }

void  *retro_get_memory_data(unsigned id)      { (void)id; return nullptr; }  // SAVE_RAM = high scores (Fase 6)
size_t retro_get_memory_size(unsigned id)      { (void)id; return 0; }

void   retro_cheat_reset(void)                 {}
void   retro_cheat_set(unsigned, bool, const char *) {}

}  // extern "C"
```

Conversão de vídeo (núcleo do `video.cpp`):

```cpp
// pal565 é reconstruída apenas quando a paleta do jogo muda.
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

void convert_8_to_565(const uint8_t *src, size_t src_stride,
                      const uint16_t *pal565, uint16_t *dst, size_t dst_pitch_px,
                      int w, int h) {
  for (int y = 0; y < h; ++y) {
    const uint8_t *s = src + (size_t)y * src_stride;
    uint16_t *d = dst + (size_t)y * dst_pitch_px;
    for (int x = 0; x < w; ++x) d[x] = pal565[s[x]];
  }
}
```

## Apêndice B — Mapeamento de botões para os descritores

```cpp
static const struct retro_input_descriptor kDescriptors[] = {
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "Left Flipper"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "Right Flipper"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "Plunger"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Nudge Left"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "Nudge Right"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Nudge Up"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "New Game"},
  {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,"Pause"},
  {0, 0, 0, 0, nullptr}
};
// env_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, (void*)kDescriptors);
```

## Apêndice C — `tools/check-no-proprietary.sh`

```bash
#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Falha se o repositório contiver algo que pareça asset do jogo.
set -euo pipefail

fail=0
say() { echo "ERRO: $*" >&2; fail=1; }

# 1) Nomes/extensões proibidos (case-insensitive) em arquivos versionados
while IFS= read -r f; do
  case "${f,,}" in
    *.dat|*.mid|*.midi|*.sf2|*.exe|*.dll|*.bin|*.wav|*.bmp)
      say "arquivo proibido versionado: $f" ;;
    */pinball.*|*/cadet.*|pinball.*|cadet.*)
      say "nome suspeito (asset do jogo?): $f" ;;
  esac
done < <(git ls-files)

# 2) Arquivos grandes (> 2 MiB) fora de uma allowlist explícita
while IFS= read -r f; do
  [ -f "$f" ] || continue
  size=$(stat -c %s "$f" 2>/dev/null || stat -f %z "$f")
  if [ "$size" -gt 2097152 ]; then
    grep -qxF "$f" tools/large-file-allowlist.txt 2>/dev/null || say "arquivo grande não permitido: $f ($size bytes)"
  fi
done < <(git ls-files | grep -v '^third_party/')

# 3) Cabeçalho mágico do DAT [VERIFICAR o valor real em partman.cpp]
if git grep -Iq "PARTOUT(4.0)RESOURCE" -- . ':!tools/check-no-proprietary.sh' ':!third_party'; then
  say "encontrado cabeçalho de arquivo de dados do jogo"
fi

# 4) Binários (não-texto) fora de pastas conhecidas
while IFS= read -r f; do
  if ! git grep -Iq . -- "$f" 2>/dev/null; then
    case "$f" in
      third_party/*) ;;                       # submódulo: não redistribuímos
      *) say "arquivo binário inesperado: $f" ;;
    esac
  fi
done < <(git ls-files)

exit $fail
```

Observação: ajuste as regras de nomes se algum arquivo próprio legítimo colidir (ex.: `pak/PINBALL.pak/` é pasta, não arquivo).
O `package.sh` deve usar **lista positiva** de arquivos, não esta heurística.

## Apêndice D — Toolchain CMake e version script

`cmake/miyoomini.toolchain.cmake`:

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Dentro do container da toolchain, CROSS_COMPILE é definido pelo ambiente (prefixo do compilador).
if(NOT DEFINED ENV{CROSS_COMPILE})
  message(FATAL_ERROR "CROSS_COMPILE não definido: rode dentro do container da toolchain (make shell).")
endif()

set(CMAKE_C_COMPILER   $ENV{CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER $ENV{CROSS_COMPILE}g++)

# [VERIFICAR] copiar as flags exatas do MinUI para miyoomini
set(ARCH_FLAGS "-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${ARCH_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_INIT "${ARCH_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections -fno-rtti")
# -fno-rtti só se o upstream não usar RTTI/dynamic_cast [VERIFICAR]

set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-static-libstdc++ -static-libgcc -Wl,--gc-sections -Wl,-z,defs")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

`src/libretro/link.T` (version script, usado com `-Wl,--version-script=`):

```
{
  global:
    retro_*;
  local:
    *;
};
```

## Apêndice E — Regras e prompts para o Cursor

`.cursor/rules/scp-libretro.mdc`:

```
---
description: Regras do projeto SpaceCadetPinball libretro (MinUI / Miyoo Mini)
alwaysApply: true
---
- NUNCA adicionar, gerar, converter ou referenciar bytes de assets do jogo (PINBALL.DAT, CADET.DAT, MIDI, sprites, sons).
  Fixtures de teste devem ser sintéticas. Se uma tarefa parecer exigir um asset, pare e pergunte.
- O build NÃO pode ler o DAT. O DAT só é lido em runtime pelo core, a partir de retro_game_info.path.
- Código novo: C++11 (compatível com o upstream), cabeçalho "SPDX-License-Identifier: MIT", comentários em inglês.
- Não modifique third_party/SpaceCadetPinball diretamente; mudanças vão em patches/NNNN-*.patch com justificativa.
- Sem alocação dinâmica, locks, I/O ou logs no caminho quente de retro_run (exceto log em nível DEBUG).
- Só exportar símbolos retro_* (visibilidade oculta por padrão).
- Antes de abrir PR: rode tools/check-no-proprietary.sh e os testes unitários/smoke.
- Itens marcados [VERIFICAR] no IMPLEMENTATION_PLAN.md devem ser confirmados no código antes de implementar.
```

Prompts sugeridos (um por fase, colar no chat do Cursor com `@IMPLEMENTATION_PLAN.md`):

1. **Fase 0:** “Execute a Fase 0. Gere `tools/sdl-inventory.sh` e `docs/PHASE0_FINDINGS.md` com a tabela de chamadas SDL/Mix/ImGui por arquivo do upstream e a recomendação para o Gate 1. Não escreva código de produção ainda.”
2. **Fase 1:** “Implemente a Fase 1: esqueleto `retro_*`, CMake host/miyoomini, version script, tela de erro e testes smoke. Siga o Apêndice A e D.”
3. **Fase 2:** “Com base no `PHASE0_FINDINGS.md`, implemente o shim/patches para compilar o upstream sem SDL/ImGui e exponha `scp::game::init/step/shutdown`. Mostre cada patch separadamente.”
4. **Fase 3:** “Implemente `video.cpp` conforme a seção 4.5, com testes unitários da conversão e da LUT.”
5. **Fases 4–6:** um prompt por fase repetindo os critérios de aceite do plano.
6. **Fase 8:** “Faça o spike S1 vs S2 descrito na Fase 8 e entregue um relatório comparando tamanho de state, esforço e riscos antes de implementar.”

## Apêndice F — Referências

- Upstream (decompilação, MIT): <https://github.com/k4zmu2a/SpaceCadetPinball>
- MinUI (arquivado em nov/2025) e guia de paks: <https://github.com/shauninman/MinUI> · `PAKS.md`
- Toolchain Miyoo Mini: <https://github.com/shauninman/union-miyoomini-toolchain>
- LessUI (fork ativo do MinUI, documentação de cores/paks): <https://github.com/nchapman/MinUI>
- API libretro: `libretro.h` (repositório `libretro/libretro-common`) e documentação em <https://docs.libretro.com>
- Ports de referência listados no README do upstream: Wii/3DS (MaikelChan), DS/PS2/Dreamcast (Headshotnoby), Vita, Switch, Emscripten.

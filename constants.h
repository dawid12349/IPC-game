#define CH_CORRIDOR ' '
#define CH_WALL (' '|A_REVERSE)
#define CH_BUSHES '#'
#define CH_PLAYER(X) ((X) + '0')
#define CH_BEAST '*'
#define CH_CAMPSITE 'A'
#define CH_COIN 'c'
#define CH_TREASURE 't'
#define CH_LTREASURE 'T'
#define CH_DTREASURE 'D'


#define SHM_SERVERINFO "/SERVER_INFO"
#define SHM_PLAYERBUS1 "/PLAYERBUS1"
#define SHM_PLAYERBUS2 "/PLAYERBUS2"
#define SHM_PLAYERBUS3 "/PLAYERBUS3"
#define SHM_PLAYERBUS4 "/PLAYERBUS4"
#define SEM_CON "/SEM_CON"


#define MAZE_INITIALIZATOR { \
"##########################################", \
"# #   #             #   #   # # #   #   #", \
"# # ### # ############# ### # # ### # ###", \
"#     # #           # #         #   #   #",\
"# ########### ### # # # ### ### # # # ###",\
"#   # # #     # # # #   #   #   # #   # #",\
"# ### # # ### # ### ######### # # ##### #",\
"#   # # # #       # # # # #   # #       #",\
"# # # # # ####### # # # # # ##### # #####",\
"# #   #       # # #   # # #     # #   # #",\
"# # ##### ### # # ##### # # # ### ### # #",\
"# # #   # #     # #     #   #       #   #",\
"# # # ### ######### # ### ##### # ### # #",\
"# #     #   # #     # #   #     # # # # #",\
"### # # ### # # ### ### # ### ##### # # #",\
"# # # #   #       #     # #     # # # # #",\
"# # # ##### ############# # ### # # # # #",\
"#   #     #   #     # # # #   #   #   # #",\
"####### ### ####### # # # ##### ### # ###",\
"#       # #   # #   #         #   # #   #",\
"# ##### # # ### # # ### ### # # ### ### #",\
"# # # #       # # #     #   # # #     # #",\
"# # # # # ##### ####### ######### # #####",\
"#   # # #       #     # #     # # #     #",\
"# # # # ########### ##### ### # # #######",\
"# # #         # #   # # #   # #     #   #",\
"# ### # ### ### ### # # ### # ### ##### #",\
"#   # #   #   # #   #       # # # #     #",\
"# # ### # # # # ### ### ##### # # ##### #",\
"# # #   # # #       #   #               #",\
"#########################################",}



#define CAMPSITE_INITIALIZATOR {18, 14}
#define BUSHES_INITIALIZATOR {{12,6}, {11,6}, {13,6}, {37,8},{38,8},{39,8}, {9,26},{10,26},{11,26}, {31,30},{32,30},{33,30}}

#define SV_CLIENT_CNT (10)
#define SV_CLIENT_DCNT (-10)
#define SV_CLIENT_MV 97
#define SV_SERVER_DECLINE 5

#define MAX_BONUSES 20
#define MAX_BUSHES 12
#define MAX_PLAYERS 4
#define MAX_BEASTS 3

#define FOV 5

#define B_COIN 1
#define B_TREASURE 10
#define B_LTREASURE 50

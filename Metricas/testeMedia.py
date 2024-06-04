import numpy as np

# Dados fornecidos
tempos_execucao = [
    141207.1157, 162064.4724, 184849.5958, 172449.5475, 188662.1773,
    181624.5449, 190776.5410, 199876.7972, 213743.5622, 219104.5542,
    188186.9497, 166494.4801, 201141.2179, 192801.9273, 186668.8123,
    914530.8723, 968041.2948, 1018601.7921, 1117678.1747, 1148956.8503,
    809409.1678, 803760.8793, 937911.6552, 897168.6590, 896057.9474,
    912060.0393, 997359.1485, 1127019.4790, 1121780.9086, 1151978.2617
]


media = np.mean(tempos_execucao)

print(f"Média: {media:.4f} ms")
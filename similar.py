from ollama import Client
import numpy as np

# 初始化客户端，确保Ollama服务正在运行
client = Client(host='http://localhost:11434')

def cosine_similarity(vec_a, vec_b):
    """计算两个向量的余弦相似度"""
    a = np.array(vec_a)
    b = np.array(vec_b)
    return np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b))

# 准备需要比较的文本
text1 = input("文本1:")
text2 = input("文本2:")

# 通过Ollama获取文本向量
response1 = client.embeddings(model='dengcao/Qwen3-Embedding-4B:Q5_K_M', prompt=text1)
response2 = client.embeddings(model='dengcao/Qwen3-Embedding-4B:Q5_K_M', prompt=text2)

embedding1 = response1['embeddings']
embedding2 = response2['embeddings']

# 计算并打印相似度
similarity = cosine_similarity(embedding1, embedding2)
print(f"文本1: {text1}")
print(f"文本2: {text2}")
print(f"语义相似度: {similarity:.4f}")

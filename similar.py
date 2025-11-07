import requests
import json
import numpy as np
from sklearn.metrics.pairwise import cosine_similarity

def get_embedding_api(text, model_name="embeddinggemma", host="localhost", port=11434):
    """通过API获取文本嵌入向量"""
    url = f"http://{host}:{port}/api/embeddings"
    
    payload = {
        "model": model_name,
        "prompt": text
    }
    
    try:
        response = requests.post(url, json=payload)
        response.raise_for_status()
        result = response.json()
        return np.array(result['embedding'])
    except requests.exceptions.RequestException as e:
        print(f"API请求错误: {e}")
        return None

def text_similarity_demo():
    """文本相似度演示"""
    print("=== 文本相似度计算器 ===")
    print("使用EmbeddingGemma模型计算两个文本的语义相似度\n")
    
    while True:
        text1 = input("请输入第一个文本 (输入 'quit' 退出): ").strip()
        if text1.lower() == 'quit':
            break
            
        text2 = input("请输入第二个文本: ").strip()
        if text2.lower() == 'quit':
            break
        
        if not text1 or not text2:
            print("文本不能为空，请重新输入！\n")
            continue
        
        print("\n正在计算相似度...")
        
        # 获取嵌入向量
        emb1 = get_embedding_api(text1)
        emb2 = get_embedding_api(text2)
        
        if emb1 is not None and emb2 is not None:
            # 计算相似度
            similarity = cosine_similarity(emb1.reshape(1, -1), emb2.reshape(1, -1))[0][0]
            
            print(f"\n📊 结果:")
            print(f"文本1: '{text1}'")
            print(f"文本2: '{text2}'")
            print(f"余弦相似度: {similarity:.4f}")
            print(f"相似度百分比: {similarity * 100:.2f}%")
            
            # 详细解释
            print(f"\n📈 相似度分析:")
            if similarity > 0.9:
                print("🎯 语义几乎相同")
            elif similarity > 0.7:
                print("🎯 高度相似")
            elif similarity > 0.5:
                print("🎯 中等相似")
            elif similarity > 0.3:
                print("🎯 轻微相似")
            else:
                print("🎯 不相似")
                
            # 显示向量信息
            print(f"\n🔢 技术信息:")
            print(f"向量维度: {len(emb1)} 维")
            print(f"向量范数 - 文本1: {np.linalg.norm(emb1):.4f}")
            print(f"向量范数 - 文本2: {np.linalg.norm(emb2):.4f}")
        else:
            print("❌ 获取嵌入向量失败，请检查Ollama服务是否运行")
        
        print("\n" + "="*50 + "\n")

if __name__ == "__main__":
    text_similarity_demo()

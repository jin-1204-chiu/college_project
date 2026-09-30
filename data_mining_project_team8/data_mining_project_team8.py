import torch
import torch.nn as nn #載入神經網路模組
import torch.optim as optim #載入優化器
import torch.nn.functional as F # 用 F.cross_entropy 計算 cross entropy loss
# WeightedRandomSampler : 實現 Weighted Random Sampling，讓少數資料可以有較高的被抽取機率
from torch.utils.data import Dataset, DataLoader, WeightedRandomSampler #Dataset定義怎麼讀一張圖，DataLoader把圖片打包成一批一批（Batch）
from torchvision import transforms, models #transforms 圖片變形
import copy
from PIL import Image # 載入 Python 的修圖工具，用來打開 .jpg 圖片檔
import pandas as pd #用來處理表格資料（CSV）和數字運算
import os
from sklearn.model_selection import train_test_split # 可將資料集依據指定的比例，隨機地切割成兩個或多個子集，如訓練集和驗證集，實現分層抽樣 (Stratified Sampling)
from sklearn.metrics import confusion_matrix # 計算模型準確度

# 參數設定
DATA_DIR = r"D:\project_AI_sam2\data_mining\butterfly_dataset"  # 訓練 + 測試資料路徑
TRAIN_RATIO = 0.8  # 訓練集 80%
VAL_RATIO = 0.1    # 驗證集 10%，用來調整參數
TEST_RATIO = 0.1   # 內部測試集 10%，用來算訓練結果的準確率

BATCH_SIZE = 8
ACCUMULATION_STEPS = 4
IMG_SIZE = 224
NUM_EPOCHS = 15 
BEST_THRESHOLD = 0.60 

# 連接GPU，沒有就用CPU
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
print(f"使用設備: {device}")

# 資料集處理
class ButterflyDataset(Dataset):
    def __init__(self, file_list, transform=None):
        self.data = file_list
        self.transform = transform
    def __len__(self): 
        return len(self.data) #告訴 PyTorch 這個資料集總共有幾張圖
    def __getitem__(self, idx):
        img_path, label = self.data[idx] # 用 index 代表對應圖片
        try:
            image = Image.open(img_path).convert('RGB')
        except:
            image = Image.new('RGB', (IMG_SIZE, IMG_SIZE))
        if self.transform:
            image = self.transform(image)
        return image, label

# 測試資料集類別
class ButterflyTestDataset(Dataset):
    def __init__(self, test_dir, transform=None):
        self.transform = transform
        self.filenames = sorted([f for f in os.listdir(test_dir) if f.endswith('.jpg')])
        self.filepaths = [os.path.join(test_dir, f) for f in self.filenames]
    def __len__(self): 
        return len(self.filepaths)
    def __getitem__(self, idx):
        img_path = self.filepaths[idx]
        image = Image.open(img_path).convert('RGB')
        if self.transform: image = self.transform(image)
        return image, self.filenames[idx]

# 用於處理資料不平衡問題的損失函數，讓稀有類別有較高的被抽取機率，並且降低信心度高的樣本的權重
class FocalLoss(nn.Module):
    def __init__(self, alpha=1, gamma=3, reduction='mean'):
        super(FocalLoss, self).__init__()
        self.alpha = alpha
        self.gamma = gamma
        self.reduction = reduction
    def forward(self, inputs, targets):
        CE_loss = F.cross_entropy(inputs, targets, reduction='none')
        pt = torch.exp(-CE_loss)
        F_loss = self.alpha * (1 - pt)**self.gamma * CE_loss
        if self.reduction == 'mean': return torch.mean(F_loss)
        else: return F_loss

# 根據資料集是訓練集或測試/驗證集，使用不同的影像前處理和資料增強步驟
def get_transforms(is_train=True):
    if is_train: # 訓練集
        return transforms.Compose([
            transforms.Resize((IMG_SIZE, IMG_SIZE)),
            transforms.RandomHorizontalFlip(), # 隨機水平翻轉
            transforms.RandomVerticalFlip(), # 隨機垂直翻轉
            transforms.RandomRotation(15), # 隨機將影像在 +/- 15 度範圍內旋轉
            transforms.RandomAffine(degrees=0, translate=(0.1, 0.1), scale=(0.9, 1.1)), # 增加 RandomAffine (隨機仿射變換) 以更好地處理不同角度或變形
            transforms.ColorJitter(brightness=0.2, contrast=0.2, saturation=0.2, hue=0.1), # 隨機改一點亮度、對比、顏色
            transforms.ToTensor(), # PIL 格式(0-255)轉換為 PyTorch 的 Tensor(0-1)
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]) # 使用平均值和標準差對影像的每個顏色通道進行標準化，讓模型訓練更快、更穩定
        ])
    else: # 測試/驗證集
        return transforms.Compose([
            transforms.Resize((IMG_SIZE, IMG_SIZE)), # 圖片大小調整成與訓練集一致
            transforms.ToTensor(), # PIL 格式(0-255)轉換為 PyTorch 的 Tensor(0-1)
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]) # 使用平均值和標準差對影像的每個顏色通道進行標準化，讓模型訓練更快、更穩定
        ])

# 模型訓練
def train_model(train_data, val_data, epochs=15, save_name='best_model.pth'):
    # 訓練所需的資料集載入(training + validation)
    train_ds = ButterflyDataset(train_data, transform=get_transforms(is_train=True))
    val_ds = ButterflyDataset(val_data, transform=get_transforms(is_train=False))
    
    # 建立訓練及分類答案的清單，hybrid 為 0 ， non-hybrid 為 1
    targets = [l for _, l in train_ds.data]
    # 計算targets中 0 和 1 的數量，也就是hybrid和non-hybrid的數量
    class_count = [targets.count(0), targets.count(1)]

    # 計算類別權重，並防止分母為 0 (分母加一個小數字 1e-5 )
    weights = [1./(c+1e-5) for c in class_count] 

    # 計算樣本權重與建立採樣器
    sample_weights = [weights[l] for l in targets]
    # 允許重複抽取稀有類別的樣本，實現過採樣，讓模型在訓練過程中看到更多稀有樣本，克服資料不平衡 ( replacement=True )
    sampler = WeightedRandomSampler(sample_weights, num_samples=len(sample_weights), replacement=True)
    
    train_loader = DataLoader(train_ds, batch_size=BATCH_SIZE, sampler=sampler, num_workers=0)
    val_loader = DataLoader(val_ds, batch_size=BATCH_SIZE, shuffle=False, num_workers=0)
    
    # 模型初始化
    model = models.resnet50(weights=models.ResNet50_Weights.IMAGENET1K_V1)
    model.fc = nn.Linear(model.fc.in_features, 2)
    model = model.to(device)
    
    # 損失函數設定
    criterion = FocalLoss(alpha=1, gamma=3.0)
    scaler = torch.amp.GradScaler(device.type, enabled=(device.type == 'cuda')) # 允許模型在訓練時混合使用 32 位元和 16 位元浮點數，訓練速度更快
    
    # 直接微調
    for param in model.parameters(): param.requires_grad = False # 全部凍結，預設先將模型中所有參數的梯度計算關閉。
    for param in model.layer3.parameters(): param.requires_grad = True # 解凍 Layer 3
    for param in model.layer4.parameters(): param.requires_grad = True # 解凍 Layer 4
    for param in model.fc.parameters(): param.requires_grad = True # # 解凍並優化新添加的參數 (model.fc)
    
    # 優化器設定
    optimizer = optim.Adam(filter(lambda p: p.requires_grad, model.parameters()), lr=1e-5)
    
    best_val_score = 0.0 # 儲存訓練過程中在驗證集達到的最高性能分數
    best_model_wts = copy.deepcopy(model.state_dict()) # 儲存對應 best_val_score 的模型權重

    for epoch in range(epochs):
        model.train()
        running_loss = 0.0
        optimizer.zero_grad() # 清空舊梯度，確保每次優化時，梯度不會累積前一次 Batch 的影響
        
        # 迭代 Batch
        for i, (inputs, labels) in enumerate(train_loader):
            inputs, labels = inputs.to(device), labels.to(device) # 資料傳輸至 GPU
            with torch.amp.autocast(device.type, enabled=(device.type == 'cuda')): # 啟用 AMP，讓 PyTorch 自動判斷哪些計算可以使用更快的 16 位元精度，加快模型訓練
                outputs = model(inputs)
                loss = criterion(outputs, labels) / ACCUMULATION_STEPS
            scaler.scale(loss).backward()
            
            # 將 Batch 的實際損失累積起來，用於計算 Epoch 結束時的平均損失
            if (i+1)%ACCUMULATION_STEPS == 0:
                scaler.step(optimizer)
                scaler.update()
                optimizer.zero_grad()
            running_loss += loss.item() * ACCUMULATION_STEPS * inputs.size(0)
            
        # 驗證
        model.eval()
        tn, fp, fn, tp = 0, 0, 0, 0
        with torch.no_grad(): # 在驗證期間，禁用梯度計算以節省 GPU 記憶體並加快推理速度
            for v_in, v_lbl in val_loader: # 從 val_loader 中取出 Batch 資料 (v_in：輸入影像，v_lbl：真實標籤)
                v_in, v_lbl = v_in.to(device), v_lbl.to(device)
                v_out = model(v_in)
                v_prob = torch.softmax(v_out, dim=1)[:, 1] # 將 Logits 轉換為機率，並取出 non-hybrid (標籤 1) 的預測機率
                v_pred = (v_prob >= BEST_THRESHOLD).long()
                
                # 遍歷 Batch 中的每個預測值 (p) 和真實標籤 (t)，並更新混淆矩陣的計數器
                for p, t in zip(v_pred, v_lbl):
                    if t==0: 
                        if p==0: tn+=1 
                        else: fp+=1
                    else:
                        if p==1: tp+=1 
                        else: fn+=1
        # 對應 TNR ，標籤 0 的召回率 (hybrid)
        hybrid_acc = tn/(tn+fp) if (tn+fp)>0 else 0
        # 對應 TPR ，標籤 1 的召回率 (non-hybrid)
        non_acc = tp/(tp+fn) if (tp+fn)>0 else 0
        
        print(f"Epoch {epoch+1}/{epochs} | Val Hybrid: {hybrid_acc:.1%} | Val Non: {non_acc:.1%}")
        
        # 存檔條件：Non 夠高(non_acc >= 95%)，Hybrid 越高越好
        if non_acc >= 0.95 and hybrid_acc >= best_val_score:
            best_val_score = hybrid_acc
            best_model_wts = copy.deepcopy(model.state_dict())
    
    # 載入最好的權重並存檔
    model.load_state_dict(best_model_wts)
    torch.save(model.state_dict(), save_name)
    return model

def evaluate_on_test(model, test_data, threshold=0.6):
    ds = ButterflyDataset(test_data, transform=get_transforms(is_train=False))
    loader = DataLoader(ds, batch_size=BATCH_SIZE, shuffle=False, num_workers=0)
    model.eval()
    
    # 儲存所有樣本的真實標籤 (y_true) 和模型預測標籤 (y_pred)
    y_true = []
    y_pred = []
    
    with torch.no_grad():
        for inputs, labels in loader:
            inputs = inputs.to(device)
            outputs = model(inputs)
            probs = torch.softmax(outputs, dim=1)[:, 1] # 計算 Non-Hybrid 機率
            preds = (probs >= threshold).long()
            y_true.extend(labels.numpy())
            y_pred.extend(preds.cpu().numpy())
            
    cm = confusion_matrix(y_true, y_pred, labels=[0, 1]) # 計算混淆矩陣
    tn, fp, fn, tp = cm.ravel()
    
    print("\n📊 內部測試集報告 (Internal Test Report):")
    print(f"Confusion Matrix:\n{cm}")
    print(f"Hybrid Acc (TNR): {tn/(tn+fp):.2%} ({tn}/{tn+fp})")
    print(f"Non-Hybrid Acc (TPR): {tp/(tp+fn):.2%} ({tp}/{tp+fn})")
    return tn/(tn+fp), tp/(tp+fn)

# 主流程
if __name__ == '__main__':
    # 準備資料
    TRAIN_HYBRID_DIR = os.path.join(DATA_DIR, 'train', 'hybrid')
    TRAIN_NONHYBRID_DIR = os.path.join(DATA_DIR, 'train', 'non-hybrid')
    TEST_DIR_TEACHER = os.path.join(DATA_DIR, 'test')
    
    hybrid_files = [(os.path.join(TRAIN_HYBRID_DIR, f), 0) for f in os.listdir(TRAIN_HYBRID_DIR) if f.endswith('.jpg')]
    nonhybrid_files = [(os.path.join(TRAIN_NONHYBRID_DIR, f), 1) for f in os.listdir(TRAIN_NONHYBRID_DIR) if f.endswith('.jpg')]
    all_data = hybrid_files + nonhybrid_files
    all_labels = [l for _, l in all_data]

    # Step 1: 三方切分 (Train/Val/Internal_Test)
    print("\n=== Phase 1: 切分 Train / Val / Internal_Test ===")
    
    # 先切出 10% 當 Internal Test
    train_val_data, test_data, train_val_labels, test_labels = train_test_split(
        all_data, all_labels, test_size=TEST_RATIO, stratify=all_labels, random_state=42
    )
    
    # 切出 10% 當 Validation，剩下的 80% 當 Train
    val_split_adjusted = VAL_RATIO / (1 - TEST_RATIO) 
    train_data, val_data, train_labels, val_labels = train_test_split(
        train_val_data, train_val_labels, test_size=val_split_adjusted, stratify=train_val_labels, random_state=42
    )

    print(f"Train: {len(train_data)}, Val: {len(val_data)}, Internal Test: {len(test_data)}")
    
    print("\n=> 開始訓練 ")
    best_model_phase1 = train_model(train_data, val_data, epochs=NUM_EPOCHS, save_name='model_phase1_gamma3.pth')
    
    print("\n=> 利用 Internal Test 評估模型準確度 ")
    evaluate_on_test(best_model_phase1, test_data, threshold=BEST_THRESHOLD)

    # Step 2: 全量重訓 (Train + Val + Internal_Test)
    print("\n=== Phase 2: 全資料重訓 (Train + Val + Internal_Test) ===")

    # 在 Phase 1 中，已經利用訓練集和驗證集來調整超參數
    # 在 Phase 2 中， 為了讓模型學習到最多的特徵，將 Phase 1 中切分出去的驗證集和內部測試集全部加回訓練
    final_model = train_model(all_data, val_data, epochs=NUM_EPOCHS, save_name='final_model_retrained_gamma3.pth') 

    # Step 3: 最終預測
    print("\n=== Phase 3: 預測蝴蝶檔案的 Test Set ===")
    final_model.eval()
    test_ds = ButterflyTestDataset(TEST_DIR_TEACHER, transform=get_transforms(is_train=False))
    test_loader = DataLoader(test_ds, batch_size=BATCH_SIZE, shuffle=False, num_workers=0)
    
    results = []
    with torch.no_grad():
        for inputs, filenames in test_loader:
            inputs = inputs.to(device)
            outputs = final_model(inputs)
            probs = torch.softmax(outputs, dim=1)[:, 1]
            preds = (probs >= BEST_THRESHOLD).long()
            for f, p in zip(filenames, preds.cpu().numpy()):
                results.append([f, p])
                
    df = pd.DataFrame(results, columns=['filename', 'label'])
    df.to_csv('Final_Submission_gamma3.csv', index=False)
    print("✅ 最終結果已儲存: Final_Submission_gamma3.csv")
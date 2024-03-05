### 算子注册说明

----

```json
  "样例": [
    {
      "state": "init",
      "superName": "样例",
      "name": "example",
      "dllPath":"resources/path/to/dllpath/",
      "roiUI": {
        "type": ["rect", "circle", "polygon", "lineCaliper", "circleCaliper"]
      },
      "resultShowUI": {
        "结果判断": [
          {
            "label": "检测区域宽度判断",
            "param": "detect_area.width",
            "type": "double"
          }
        ],
        "图像显示": [
          {
            "label": "检测区域",
            "param": "detect_area"
          }
        ],
        "文本显示": []
      },
      "outPara": [
        {
          "varLabel": "模块状态",
          "varName": "status",
          "varType": "int",
          "currentDataFlag": true
        },
        {
          "varLabel": "检测区域",
          "varName": "detect_area",
          "varType": "GeliRect",
          "children": [
            {
              "varLabel": "检测区域中心点",
              "varName": "center",
              "varType": "GeliPoint",
              "children": [
                {
                  "varLabel": "检测区域中心点X",
                  "varName": "x",
                  "varType": "double"
                },
                {
                  "varLabel": "检测区域中心点Y",
                  "varName": "y",
                  "varType": "double"
                }
              ]
            },
            {
              "varLabel": "检测区域宽度",
              "varName": "width",
              "varType": "double"
            },
            {
              "varLabel": "检测区域高度",
              "varName": "height",
              "varType": "double"
            },
            {
              "varLabel": "检测区域角度",
              "varName": "angle",
              "varType": "double"
            }
          ]
        }
      ],
      "models": [
        {
          "modelName": "model1",
          "helpMsg": "it is a help msg of model1",
          "dllName": "*",
          "modelID": "func_name1",
          "inPara": [
            {
              "varLabel": "输出源",
              "varName": "输出源",
              "varType": "int",
              "fromExpression": "0",
              "defineVarInputWay": "selectedInputWay",
              "comboList": {
                "数据队列": "0",
                "通信设备": "1",
                "全局变量": "2",
                "视觉控制器": "3",
                "发送事件": "4"
              },
              "varExplanation": ""
            }
          ]
        }
      ]
    }
  ],
```



| 名称                  | 含义                                                         |
| --------------------- | ------------------------------------------------------------ |
| state                 | 固定为init,表示算子状态                                      |
| superName             | 上一级菜单名称                                               |
| name                  | 算子名称                                                     |
| dllPath               | dll路径所在文件夹,不含dll文件                                |
| roiUI.type            | 绘制roi区域，目前可选"rect", "circle", "polygon", "lineCaliper", "circleCaliper" |
| resultShowUI          | 绘制结果显示区                                               |
| resultShowUI.结果判断 | 可选，绘制结果显示区的结果判断区域，label为显示标签；param值和type值与outParam的某个对应输出对应 |
| resultShowUI.图像显示 | 可选，绘制结果显示区的图像显示区域                           |
| resultShowUI.文本显示 | 可选，绘制结果显示区的文本显示区域，固定为"文本显示":[]      |
| models                | 里面每一个model就是算子的每一种方法，如滤波里面可能有高斯、均值滤波... |
| outPara               | 算子输出参数，注意每种类型格式固定，仅仅可修改所以的varLabel和最外层的varName,如对于GeliRect来说，仅仅只可修改"varName": "detect_area"的值，和所有内外层的varLabel的值。 |

| mode内部属性名称 | 含义            |
| ---------------- | --------------- |
| dllName          | dll名，不含.dll |
| modelName        | 方法中文名称    |
| helpMsg          | 帮助信息        |
| modelID          | 方法名          |
| inPara           | 入参            |

| 参数内部属性名称  | 含义                                                         |
| ----------------- | ------------------------------------------------------------ |
| varLabel          | 参数中文名                                                   |
| varName           | 参数标识符                                                   |
| varType           | 参数类型，  类型仅仅只能用int, double, string, 还有object.h里面的Geli...类型 |
| currentDataFlag   | 仅出参，可选，是否需要显示到当前结果区域标志                 |
| fromExpression    | 仅入参，默认值                                               |
| defineVarInputWay | 仅入参，directInputWay 直接输入框\|selectedInputWay字面值下拉选择框\|smartInputWay其他算子的结果作为源头数据下拉选择框 |
| comboList         | 仅入参，若为selectedInputWay需要定义                         |
| varExplanation    | 描述信息                                                     |
| low               | 仅入参，可选，下限                                           |
| high              | 仅入参，可选，上限                                           |
| step              | 仅入参，可选，步长                                           |
| precision         | 仅入参，可选，精度，小数点后位数                             |

1. 参考[GeliVision/src/流程图操作/js/newOperatorLib.json at master · zwqgkd/GeliVision (github.com)](https://github.com/zwqgkd/GeliVision/blob/master/src/流程图操作/js/newOperatorLib.json)
2. 添加算子的结构到相应位子，运行项目可以看到算子已经在左侧算子选择栏
3. 拖动算子，双击打开配置界面，查看界面是否符合要求，大部分常规界面可以依赖这个json自动渲染想要的配置界面（一个静态的表单，一个roi区域，一个结果显示标签页）

### 算子封dll说明

---

```C++
extern "C" __declspec(dllexport) void gaussian_blur(ParamPtrArray& params) {
    // get in_params
    GeliMat in_param0_mat = get_data_ref<GeliMat>(params[0]);
    int     in_param1_size = get_data<int>(params[1]);
    // 如果算子有roi区域，那么roi相关数据会push到params里
    Roi roi = get_data_ref<Roi>(params[2]);

    // 如果是图形类型，那么需要调用以下函数生成mask_mat
    if (!roi.mat_flag) {
        roi.generate_mask_mat(in_param0_mat);
    }

    // 第一个输出，算子状态
    int status=static_cast<int>(OperatorStatus::OK) ;
    cv::Mat resultImage;
    try {
        cv::GaussianBlur(in_param0_mat.mat, resultImage,
            cv::Size(in_param1_size, in_param1_size), 0, 0,
            cv::BORDER_DEFAULT);
        cv::Mat maskedResult;
        in_param0_mat.mat.copyTo(maskedResult, ~roi.mask_mat.mat);
        resultImage.copyTo(maskedResult, roi.mask_mat.mat);
    }
    catch (std::exception& e) {
        status = static_cast<int>(OperatorStatus::NG);
        throw ExecutionFailedException("执行错误", "高斯滤波执行错误");
    }
    params.push_back(make_param<int>(status));

    // 第二个输出，处理后的图像
    params.push_back(make_param<GeliMat>(cv::Mat(resultImage)));

    // 第三个输出，掩膜图像
    if (roi.mask) {
        // 如果需要输出掩膜图像
        params.push_back(make_param<GeliMat>(roi.mask_mat));
    }
    else {
        params.emplace_back(nullptr);
    }

    // 第四五个输出，关于区域信息：检测区域和roi圆弧
    if (roi.type == "") {
        params.emplace_back(nullptr);
        params.emplace_back(nullptr);
    }
    else if (roi.type == "rect") {
        params.push_back(make_param<GeliRect>(roi.generate_area()));
        params.emplace_back(nullptr);

    }
    else if (roi.type == "circle") {
        params.emplace_back(nullptr);
        params.push_back(make_param<GeliCircle>(roi.generate_circle()));
    }
    // 第六个输出，屏蔽区
    params.push_back(make_param<GeliShield>(roi.shield));
}
```

1. 包含头文件object.h，里面有Roi，异常类，算子状态枚举类，Geli...结构化参数类，

   [mh/NetworkManager/object.h at refactor · zwqgkd/mh (github.com)](https://github.com/zwqgkd/mh/blob/refactor/NetworkManager/object.h)

2. 算子的入参类型仅仅只能用int, double, string, 还有object.h里面的Geli...类型和Roi类型，如果算子有ROI区域，那么其参数都封装到了Roi对象里，并作为最后一个额外的入参。Roi定义在object.h中

3. 算子的出参严格按照json里的outPara的顺序与类型push_back到params的之后。

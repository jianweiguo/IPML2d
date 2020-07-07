# Neural Detector

This is a tensorflow implementation for detection of atomic structures.    
The code is based on [Faster-RCNN](https://github.com/DetectionTeamUCAS/Faster-RCNN_Tensorflow), and [R2CNN](https://github.com/DetectionTeamUCAS/R2CNN_Faster-RCNN_Tensorflow). 

## Environment
1. python2.7 (Anaconda recommend)     
2. cuda8.0     
3. tensorflow >= 1.2   
4. opencv(cv2)

## Pre-trained models
1. Download [resnet50_v1](http://download.tensorflow.org/models/resnet_v1_50_2016_08_28.tar.gz)、[resnet101_v1](http://download.tensorflow.org/models/resnet_v1_101_2016_08_28.tar.gz) and put it to data/pretrained_weights.     
2. Download [mobilenet_v2](https://storage.googleapis.com/mobilenet_v2/checkpoints/mobilenet_v2_1.0_224.tgz), put it to data/pretrained_weights/mobilenet.      

## Compile
```  
cd $PATH_ROOT/libs/box_utils/
python setup.py build_ext --inplace
```

```  
cd $PATH_ROOT/libs/box_utils/cython_utils
python setup.py build_ext --inplace
```

## Usage
1. Data format
```
├── VOCdevkit
│   ├── VOCdevkit_train
│       ├── Annotation
│       ├── JPEGImages
│    ├── VOCdevkit_test
│       ├── Annotation
│       ├── JPEGImages
```  

2. Data prepare:  
```     
(1) Modify parameters (such as CLASS_NUM, DATASET_NAME, VERSION, etc.) in $PATH_ROOT/libs/configs/cfgs.py
(2) Add category information in $PATH_ROOT/libs/label_name_dict/lable_dict.py     
(3) Add data_name to line 75 of $PATH_ROOT/data/io/read_tfrecord.py 
```     

3. Make tfrecord
```  
cd $PATH_ROOT/data/io/  
python convert_data_to_tfrecord.py --VOC_dir='/PATH/TO/VOCdevkit/VOCdevkit_train/' 
                                   --xml_dir='Annotation'
                                   --image_dir='JPEGImages'
                                   --save_name='train' 
                                   --img_format='.jpg' 
                                   --dataset='IPML2d'
```     

4. Train
```  
cd $PATH_ROOT/tools
python train.py
```

5. Inference
```  
cd $PATH_ROOT/tools
python inference.py --data_dir='inference_image/' --gpu='0'
```

## Tensorboard
```  
cd $PATH_ROOT/output/summary
tensorboard --logdir=.
``` 
#include <algorithm>
#include <cfloat>
#include <vector>

#include "caffe/layers/multi_task_softmax_loss_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void MultiTaskSoftmaxWithLossLayer<Dtype>::LayerSetUp(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::LayerSetUp(bottom, top);
  LayerParameter softmax_param(this->layer_param_);
  softmax_param.set_type("Softmax");
  softmax_layer_ = LayerRegistry<Dtype>::CreateLayer(softmax_param);
  softmax_bottom_vec_.clear();
  softmax_bottom_vec_.push_back(bottom[0]);
  softmax_top_vec_.clear();
  softmax_top_vec_.push_back(&prob_);
  softmax_layer_->SetUp(softmax_bottom_vec_, softmax_top_vec_);

  has_ignore_label_ =
    this->layer_param_.loss_param().has_ignore_label();
  if (has_ignore_label_) {
    ignore_label_ = this->layer_param_.loss_param().ignore_label();
  }
  if (!this->layer_param_.loss_param().has_normalization() &&
      this->layer_param_.loss_param().has_normalize()) {
    normalization_ = this->layer_param_.loss_param().normalize() ?
                     LossParameter_NormalizationMode_VALID :
                     LossParameter_NormalizationMode_BATCH_SIZE;
  } else {
    normalization_ = this->layer_param_.loss_param().normalization();
  }

  if (this->layer_param_.multi_task_softmax_loss_param().weight_labels_size()) {
	int weight_labels_size =
	  this->layer_param_.multi_task_softmax_loss_param().weight_labels_size();
    CHECK_EQ(weight_labels_size,
	  this->layer_param_.multi_task_softmax_loss_param().labels_weight_size())
	  << "weight_labels_size should be equal to labels_weight_size";

    std::cout << "sz : " << weight_labels_size<<std::endl;

    int labels_weights_size = prob_.channels();
    labels_weights_ = new Dtype[labels_weights_size];
    memset(labels_weights_, Dtype(1), sizeof(Dtype) * labels_weights_size);
    for (int i = 0; i < labels_weights_size; i ++)
    labels_weights_[i] = Dtype(1.0);
    for (int i = 0; i < weight_labels_size; i ++) {
      labels_weights_[this->layer_param_.multi_task_softmax_loss_param().weight_labels(i)]=
	 this->layer_param_.multi_task_softmax_loss_param().labels_weight(i);
	std::cout << "w[" << this->layer_param_.multi_task_softmax_loss_param().weight_labels(i)<<"]="<<
	labels_weights_[this->layer_param_.multi_task_softmax_loss_param().weight_labels(i)]<<std::endl;
	}
   }

}

template <typename Dtype>
void MultiTaskSoftmaxWithLossLayer<Dtype>::Reshape(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::Reshape(bottom, top);
  softmax_layer_->Reshape(softmax_bottom_vec_, softmax_top_vec_);
  softmax_axis_ =
      bottom[0]->CanonicalAxisIndex(this->layer_param_.softmax_param().axis());
  outer_num_ = bottom[0]->count(0, softmax_axis_);
  inner_num_ = bottom[0]->count(softmax_axis_ + 1);
  CHECK_EQ(outer_num_ * inner_num_, bottom[1]->count())
      << "Number of labels must match number of predictions; "
      << "e.g., if softmax axis == 1 and prediction shape is (N, C, H, W), "
      << "label count (number of labels) must be N*H*W, "
      << "with integer values in {0, 1, ..., C-1}.";
  if (top.size() >= 2) {
    // softmax output
    top[1]->ReshapeLike(*bottom[0]);
  }
}

template <typename Dtype>
Dtype MultiTaskSoftmaxWithLossLayer<Dtype>::get_normalizer(
    LossParameter_NormalizationMode normalization_mode, int valid_count) {
  Dtype normalizer;
  switch (normalization_mode) {
    case LossParameter_NormalizationMode_FULL:
      normalizer = Dtype(outer_num_ * inner_num_);
      break;
    case LossParameter_NormalizationMode_VALID:
      if (valid_count == -1) {
        normalizer = Dtype(outer_num_ * inner_num_);
      } else {
        normalizer = Dtype(valid_count);
      }
      break;
    case LossParameter_NormalizationMode_BATCH_SIZE:
      normalizer = Dtype(outer_num_);
      break;
    case LossParameter_NormalizationMode_NONE:
      normalizer = Dtype(1);
      break;
    default:
      LOG(FATAL) << "Unknown normalization mode: "
          << LossParameter_NormalizationMode_Name(normalization_mode);
  }
  // Some users will have no labels for some examples in order to 'turn off' a
  // particular loss in a multi-task setup. The max prevents NaNs in that case.
  return std::max(Dtype(1.0), normalizer);
}

template <typename Dtype>
void MultiTaskSoftmaxWithLossLayer<Dtype>::Forward_cpu(
    const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  // The forward pass computes the softmax prob values.
  softmax_layer_->Forward(softmax_bottom_vec_, softmax_top_vec_);
  const Dtype* prob_data = prob_.cpu_data();
  const Dtype* label = bottom[1]->cpu_data();
  int dim = prob_.count() / outer_num_;
  int count = 0;
  Dtype loss = 0;
  for (int i = 0; i < outer_num_; ++i) {
    for (int j = 0; j < inner_num_; j++) {
      const int label_value = static_cast<int>(label[i * inner_num_ + j]);
      if (has_ignore_label_ && label_value == ignore_label_) {
        continue;
      }
      CHECK_GE(label_value, 0)<<"negative label found error";
      CHECK_LT(label_value, prob_.shape(softmax_axis_))<<"label greater than number of chanels, error! found error";
      loss -= labels_weights_[label_value]*log(std::max(prob_data[i * dim + label_value * inner_num_ + j],
                           Dtype(FLT_MIN)));
      ++count;
    }
  }
  /*std::cout<<"shape prob: "<<prob_.shape_string ()<<std::endl;
  std::cout<<"dim: "<<dim<<std::endl;
  std::cout<<"inner: "<<inner_num_<<std::endl;
  std::cout<<"outer: "<<outer_num_<<std::endl;*/
  top[0]->mutable_cpu_data()[0] = loss / get_normalizer(normalization_, count);
  if (top.size() == 2) {
    top[1]->ShareData(prob_);
  }
}

template <typename Dtype>
void MultiTaskSoftmaxWithLossLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
  if (propagate_down[1]) {
    LOG(FATAL) << this->type()
               << " Layer cannot backpropagate to label inputs.";
  }
  if (propagate_down[0]) {
    Dtype* bottom_diff = bottom[0]->mutable_cpu_diff();
    const Dtype* prob_data = prob_.cpu_data();
    const Dtype* label = bottom[1]->cpu_data();
    int dim = prob_.count() / outer_num_;
    int count = 0;
    //caffe_copy(prob_.count(), prob_data, bottom_diff);
    // we assign w_yi * P_i
    for (int i = 0; i < outer_num_; ++i) {
      for (int j = 0; j < inner_num_; ++j) {
	const int label_value = static_cast<int>(label[i * inner_num_ + j]);
         for (int c = 0; c < bottom[0]->shape(softmax_axis_); ++c) {
            bottom_diff[i * dim + c * inner_num_ + j] =labels_weights_[label_value]*prob_data[i * dim + c * inner_num_ + j] ;
          }

	}
      }


    for (int i = 0; i < outer_num_; ++i) {
      for (int j = 0; j < inner_num_; ++j) {
        const int label_value = static_cast<int>(label[i * inner_num_ + j]);
        if (has_ignore_label_ && label_value == ignore_label_) {
          for (int c = 0; c < bottom[0]->shape(softmax_axis_); ++c) {
            bottom_diff[i * dim + c * inner_num_ + j] = 0;
          }
        } else {
          bottom_diff[i * dim + label_value * inner_num_ + j] -= labels_weights_[label_value];//now let's compute w_yi*(p_i -1)
          ++count;
        }
      }
    }
    // Scale gradient
    Dtype loss_weight = top[0]->cpu_diff()[0] /
                        get_normalizer(normalization_, count);
    caffe_scal(prob_.count(), loss_weight, bottom_diff);
  }
}

#ifdef CPU_ONLY
STUB_GPU(SoftmaxWithLossLayer);
#endif

INSTANTIATE_CLASS(MultiTaskSoftmaxWithLossLayer);
REGISTER_LAYER_CLASS(MultiTaskSoftmaxWithLoss);

}  // namespace caffe

/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file elemwise_binary_scalar_op_extended.cc
 * \brief CPU Implementation of extended binary scalar functions.
 */
#include "./elemwise_unary_op.h"
#include "./elemwise_binary_op.h"
#include "./elemwise_binary_scalar_op.h"

namespace mxnet {
namespace op {
MXNET_OPERATOR_REGISTER_BINARY_SCALAR(_maximum_scalar)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Compute<cpu, mshadow_op::maximum>)
    .set_attr<nnvm::FGradient>("FGradient", ElemwiseGradUseIn{"_backward_maximum_scalar"})
    .add_alias("_MaximumScalar")
    .add_alias("_npi_maximum_scalar");

MXNET_OPERATOR_REGISTER_BINARY(_backward_maximum_scalar)
    .add_arguments(NumpyBinaryScalarParam::__FIELDS__())
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Backward<cpu, mshadow_op::ge>);

MXNET_OPERATOR_REGISTER_BINARY_SCALAR(_minimum_scalar)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Compute<cpu, mshadow_op::minimum>)
    .set_attr<nnvm::FGradient>("FGradient", ElemwiseGradUseIn{"_backward_minimum_scalar"})
    .add_alias("_MinimumScalar")
    .add_alias("_npi_minimum_scalar");

MXNET_OPERATOR_REGISTER_BINARY(_backward_minimum_scalar)
    .add_arguments(NumpyBinaryScalarParam::__FIELDS__())
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Backward<cpu, mshadow_op::le>);

#if MXNET_USE_ONEDNN == 1
bool PowerStorageType(const nnvm::NodeAttrs& attrs,
                      const int dev_mask,
                      DispatchMode* dispatch_mode,
                      std::vector<int>* inputs,
                      std::vector<int>* outputs) {
  CHECK_EQ(inputs->size(), 1);
  CHECK_EQ(outputs->size(), 1);

  return DNNLStorageType(attrs, dev_mask, true, dispatch_mode, inputs, outputs);
}

void PowerComputeExCPU(const nnvm::NodeAttrs& attrs,
                       const OpContext& ctx,
                       const std::vector<mxnet::NDArray>& inputs,
                       const std::vector<OpReqType>& req,
                       const std::vector<mxnet::NDArray>& outputs) {
  if (SupportDNNLPower(inputs[0])) {
    DNNL_OPCHECK_INIT(false, outputs.size(), inputs, outputs);
    DNNLRun(DNNLPowerForward, attrs, ctx, inputs[0], req[0], outputs[0]);
    DNNL_OPCHECK_RUN(
        (BinaryScalarOp::Compute<cpu, mshadow_op::power>), attrs, ctx, inputs, req, outputs);
  } else {
    FallBackCompute(
        BinaryScalarOp::Compute<cpu, mshadow_op::power>, attrs, ctx, inputs, req, outputs);
  }
}
#endif  // MXNET_USE_ONEDNN == 1

MXNET_OPERATOR_REGISTER_BINARY_SCALAR(_power_scalar)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Compute<cpu, mshadow_op::power>)
#if MXNET_USE_ONEDNN == 1
    .set_attr<FComputeEx>("FComputeEx<cpu>", PowerComputeExCPU)
    .set_attr<FInferStorageType>("FInferStorageType", PowerStorageType)
#endif
    .set_attr<nnvm::FGradient>("FGradient", ElemwiseGradUseIn{"_backward_power_scalar"})
    .add_alias("_PowerScalar");

MXNET_OPERATOR_REGISTER_BINARY(_backward_power_scalar)
    .add_arguments(NumpyBinaryScalarParam::__FIELDS__())
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Backward<cpu, mshadow_op::power_grad>);

MXNET_OPERATOR_REGISTER_BINARY_SCALAR(_rpower_scalar)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Compute<cpu, mshadow_op::rpower>)
    .set_attr<nnvm::FGradient>("FGradient", ElemwiseGradUseOut{"_backward_rpower_scalar"})
    .add_alias("_RPowerScalar");

MXNET_OPERATOR_REGISTER_BINARY(_backward_rpower_scalar)
    .add_arguments(NumpyBinaryScalarParam::__FIELDS__())
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Backward<cpu, mshadow_op::rpower_grad>);

MXNET_OPERATOR_REGISTER_BINARY_SCALAR(_hypot_scalar)
    .add_alias("_npi_hypot_scalar")
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Compute<cpu, mshadow_op::hypot>)
    .set_attr<nnvm::FGradient>("FGradient", ElemwiseGradUseIn{"_backward_hypot_scalar"})
    .add_alias("_HypotScalar");

MXNET_OPERATOR_REGISTER_BINARY(_backward_hypot_scalar)
    .add_arguments(NumpyBinaryScalarParam::__FIELDS__())
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<FCompute>("FCompute<cpu>",
                        BinaryScalarOp::Backward<cpu, mshadow_op::hypot_grad_left>);

NNVM_REGISTER_OP(smooth_l1)
    .add_alias("_npx_smooth_l1")
    .describe(R"code(Calculate Smooth L1 Loss(lhs, scalar) by summing

.. math::

    f(x) =
    \begin{cases}
    (\sigma x)^2/2,& \text{if }x < 1/\sigma^2\\
    |x|-0.5/\sigma^2,& \text{otherwise}
    \end{cases}

where :math:`x` is an element of the tensor *lhs* and :math:`\sigma` is the scalar.

Example::

  smooth_l1([1, 2, 3, 4]) = [0.5, 1.5, 2.5, 3.5]
  smooth_l1([1, 2, 3, 4], scalar=1) = [0.5, 1.5, 2.5, 3.5]

)code" ADD_FILELINE)
    .set_num_inputs(1)
    .set_num_outputs(1)
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<mxnet::FInferShape>("FInferShape", ElemwiseShape<1, 1>)
    .set_attr<nnvm::FInferType>("FInferType", ElemwiseType<1, 1>)
    .set_attr<nnvm::FInplaceOption>("FInplaceOption",
                                    [](const NodeAttrs& attrs) {
                                      return std::vector<std::pair<int, int> >{{0, 0}};
                                    })
    .add_argument("data", "NDArray-or-Symbol", "source input")
    .add_argument("scalar", "float", "scalar input")
    .set_attr<FCompute>("FCompute<cpu>", BinaryScalarOp::Compute<cpu, mshadow_op::smooth_l1_loss>)
    .set_attr<nnvm::FGradient>("FGradient", ElemwiseGradUseIn{"_backward_smooth_l1"});

MXNET_OPERATOR_REGISTER_BINARY(_backward_smooth_l1)
    .set_attr_parser(ParamParser<NumpyBinaryScalarParam>)
    .set_attr<FCompute>("FCompute<cpu>",
                        BinaryScalarOp::Backward<cpu, mshadow_op::smooth_l1_gradient>);

}  // namespace op
}  // namespace mxnet

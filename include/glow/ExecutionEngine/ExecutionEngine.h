#ifndef GLOW_EXECUTIONENGINE_EXECUTIONENGINE_H
#define GLOW_EXECUTIONENGINE_EXECUTIONENGINE_H

#include "glow/Backends/Backend.h"
#include "glow/Base/Train.h"
#include "glow/Optimizer/Optimizer.h"

#include "llvm/ADT/ArrayRef.h"

#include <memory>
#include <unordered_map>

namespace glow {

class Graph;
class Node;
class Interpreter;
class Variable;
class Tensor;
class Value;

/// This is the ExecutionEngine. It owns the Graph, the IR, and the backends.
/// The Graph, Module, etc in this class are defined as pointers, in order to
/// erase the type and prevent the internal types from leaking out to the
/// users of this class.
class ExecutionEngine final {
  /// The Graph that represents the high-level program.
  std::unique_ptr<Graph> G_;
  /// The Module that holds the IR.
  std::unique_ptr<Module> M_;
  /// The network interpreter
  std::unique_ptr<Backend> IP_;
  /// The training configuration.
  TrainingConfig config_;

public:
  ExecutionEngine(BackendKind backendKind = BackendKind::Interpreter);

  ~ExecutionEngine();

  /// \returns the internal module.
  Module &getModule() { return *M_; }

  /// \returns the internal module.
  Graph &getGraph() { return *G_; }

  /// Optimize the graph, generate IR, and optimize the IR.
  void compile(CompilationMode mode);

  /// Provides access to the training configuration.
  TrainingConfig &getConfig() { return config_; }

  /// Runs the program in a forward pass. Update the nodes in \p nodes with the
  /// values \p inputs.
  void run(llvm::ArrayRef<Variable *> vars, llvm::ArrayRef<Tensor *> inputs);

  /// Train the network. Perform \p iterations in the training loop. Each
  /// iteration does a full forward and backward pass of a whole batch.
  /// The method updates the variables in \p vars with the tensors \p inputs.
  void runBatch(size_t iterations, llvm::ArrayRef<Variable *> vars,
                llvm::ArrayRef<Tensor *> inputs);

  /// \returns a pointer to the tensor that is stored at \p v.
  Tensor *getWeight(const Variable *v) const;

private:
  /// Update the inputs for all variables \p vars with data from the inputs \p
  /// inputs at offset \p sampleIdx. Then perform a forward and backwards scan.
  void updateForwardBackward(llvm::ArrayRef<Variable *> vars,
                             llvm::ArrayRef<Tensor *> inputs, size_t sampleIdx);

  /// Update the content of the tensor \p v with some slices that from \p input.
  /// The data starts at slice \p sampleIdx and wraps around until the
  /// data in \p v is filled. All dimensions, except for the first (batch)
  /// dimension must be identical.
  void loadValueFromTensorSlice(const Variable *v, Tensor *input,
                                size_t sampleIdx);

  // Update the content of the tensor \p v with \p input.
  void loadValueFromTensor(const Variable *v, Tensor *input);
};

} // namespace glow

#endif // GLOW_EXECUTIONENGINE_EXECUTIONENGINE_H

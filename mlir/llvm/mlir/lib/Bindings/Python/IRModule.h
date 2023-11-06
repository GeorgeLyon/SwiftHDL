//===- IRModules.h - IR Submodules of pybind module -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_BINDINGS_PYTHON_IRMODULES_H
#define MLIR_BINDINGS_PYTHON_IRMODULES_H

#include <optional>
#include <utility>
#include <vector>

#include "Globals.h"
#include "PybindUtils.h"

#include "mlir-c/AffineExpr.h"
#include "mlir-c/AffineMap.h"
#include "mlir-c/Diagnostics.h"
#include "mlir-c/IR.h"
#include "mlir-c/IntegerSet.h"
#include "mlir/Bindings/Python/PybindAdaptors.h"
#include "llvm/ADT/DenseMap.h"

namespace mlir {
namespace python {

class PyBlock;
class PyDiagnostic;
class PyDiagnosticHandler;
class PyInsertionPoint;
class PyLocation;
class DefaultingPyLocation;
class PyMlirContext;
class DefaultingPyMlirContext;
class PyModule;
class PyOperation;
class PyOperationBase;
class PyType;
class PySymbolTable;
class PyValue;

/// Template for a reference to a concrete type which captures a python
/// reference to its underlying python object.
template <typename T>
class PyObjectRef {
public:
  PyObjectRef(T *referrent, pybind11::object object)
      : referrent(referrent), object(std::move(object)) {
    assert(this->referrent &&
           "cannot construct PyObjectRef with null referrent");
    assert(this->object && "cannot construct PyObjectRef with null object");
  }
  PyObjectRef(PyObjectRef &&other)
      : referrent(other.referrent), object(std::move(other.object)) {
    other.referrent = nullptr;
    assert(!other.object);
  }
  PyObjectRef(const PyObjectRef &other)
      : referrent(other.referrent), object(other.object /* copies */) {}
  ~PyObjectRef() = default;

  int getRefCount() {
    if (!object)
      return 0;
    return object.ref_count();
  }

  /// Releases the object held by this instance, returning it.
  /// This is the proper thing to return from a function that wants to return
  /// the reference. Note that this does not work from initializers.
  pybind11::object releaseObject() {
    assert(referrent && object);
    referrent = nullptr;
    auto stolen = std::move(object);
    return stolen;
  }

  T *get() { return referrent; }
  T *operator->() {
    assert(referrent && object);
    return referrent;
  }
  pybind11::object getObject() {
    assert(referrent && object);
    return object;
  }
  operator bool() const { return referrent && object; }

private:
  T *referrent;
  pybind11::object object;
};

/// Tracks an entry in the thread context stack. New entries are pushed onto
/// here for each with block that activates a new InsertionPoint, Context or
/// Location.
///
/// Pushing either a Location or InsertionPoint also pushes its associated
/// Context. Pushing a Context will not modify the Location or InsertionPoint
/// unless if they are from a different context, in which case, they are
/// cleared.
class PyThreadContextEntry {
public:
  enum class FrameKind {
    Context,
    InsertionPoint,
    Location,
  };

  PyThreadContextEntry(FrameKind frameKind, pybind11::object context,
                       pybind11::object insertionPoint,
                       pybind11::object location)
      : context(std::move(context)), insertionPoint(std::move(insertionPoint)),
        location(std::move(location)), frameKind(frameKind) {}

  /// Gets the top of stack context and return nullptr if not defined.
  static PyMlirContext *getDefaultContext();

  /// Gets the top of stack insertion point and return nullptr if not defined.
  static PyInsertionPoint *getDefaultInsertionPoint();

  /// Gets the top of stack location and returns nullptr if not defined.
  static PyLocation *getDefaultLocation();

  PyMlirContext *getContext();
  PyInsertionPoint *getInsertionPoint();
  PyLocation *getLocation();
  FrameKind getFrameKind() { return frameKind; }

  /// Stack management.
  static PyThreadContextEntry *getTopOfStack();
  static pybind11::object pushContext(PyMlirContext &context);
  static void popContext(PyMlirContext &context);
  static pybind11::object pushInsertionPoint(PyInsertionPoint &insertionPoint);
  static void popInsertionPoint(PyInsertionPoint &insertionPoint);
  static pybind11::object pushLocation(PyLocation &location);
  static void popLocation(PyLocation &location);

  /// Gets the thread local stack.
  static std::vector<PyThreadContextEntry> &getStack();

private:
  static void push(FrameKind frameKind, pybind11::object context,
                   pybind11::object insertionPoint, pybind11::object location);

  /// An object reference to the PyContext.
  pybind11::object context;
  /// An object reference to the current insertion point.
  pybind11::object insertionPoint;
  /// An object reference to the current location.
  pybind11::object location;
  // The kind of push that was performed.
  FrameKind frameKind;
};

/// Wrapper around MlirContext.
using PyMlirContextRef = PyObjectRef<PyMlirContext>;
class PyMlirContext {
public:
  PyMlirContext() = delete;
  PyMlirContext(const PyMlirContext &) = delete;
  PyMlirContext(PyMlirContext &&) = delete;

  /// For the case of a python __init__ (py::init) method, pybind11 is quite
  /// strict about needing to return a pointer that is not yet associated to
  /// an py::object. Since the forContext() method acts like a pool, possibly
  /// returning a recycled context, it does not satisfy this need. The usual
  /// way in python to accomplish such a thing is to override __new__, but
  /// that is also not supported by pybind11. Instead, we use this entry
  /// point which always constructs a fresh context (which cannot alias an
  /// existing one because it is fresh).
  static PyMlirContext *createNewContextForInit();

  /// Returns a context reference for the singleton PyMlirContext wrapper for
  /// the given context.
  static PyMlirContextRef forContext(MlirContext context);
  ~PyMlirContext();

  /// Accesses the underlying MlirContext.
  MlirContext get() { return context; }

  /// Gets a strong reference to this context, which will ensure it is kept
  /// alive for the life of the reference.
  PyMlirContextRef getRef() {
    return PyMlirContextRef(this, pybind11::cast(this));
  }

  /// Gets a capsule wrapping the void* within the MlirContext.
  pybind11::object getCapsule();

  /// Creates a PyMlirContext from the MlirContext wrapped by a capsule.
  /// Note that PyMlirContext instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirContext
  /// is taken by calling this function.
  static pybind11::object createFromCapsule(pybind11::object capsule);

  /// Gets the count of live context objects. Used for testing.
  static size_t getLiveCount();

  /// Gets the count of live operations associated with this context.
  /// Used for testing.
  size_t getLiveOperationCount();

  /// Clears the live operations map, returning the number of entries which were
  /// invalidated. To be used as a safety mechanism so that API end-users can't
  /// corrupt by holding references they shouldn't have accessed in the first
  /// place.
  size_t clearLiveOperations();

  /// Removes an operation from the live operations map and sets it invalid.
  /// This is useful for when some non-bindings code destroys the operation and
  /// the bindings need to made aware. For example, in the case when pass
  /// manager is run.
  void clearOperation(MlirOperation op);

  /// Clears all operations nested inside the given op using
  /// `clearOperation(MlirOperation)`.
  void clearOperationsInside(PyOperationBase &op);

  /// Gets the count of live modules associated with this context.
  /// Used for testing.
  size_t getLiveModuleCount();

  /// Enter and exit the context manager.
  pybind11::object contextEnter();
  void contextExit(const pybind11::object &excType,
                   const pybind11::object &excVal,
                   const pybind11::object &excTb);

  /// Attaches a Python callback as a diagnostic handler, returning a
  /// registration object (internally a PyDiagnosticHandler).
  pybind11::object attachDiagnosticHandler(pybind11::object callback);

  /// Controls whether error diagnostics should be propagated to diagnostic
  /// handlers, instead of being captured by `ErrorCapture`.
  void setEmitErrorDiagnostics(bool value) { emitErrorDiagnostics = value; }
  struct ErrorCapture;

private:
  PyMlirContext(MlirContext context);
  // Interns the mapping of live MlirContext::ptr to PyMlirContext instances,
  // preserving the relationship that an MlirContext maps to a single
  // PyMlirContext wrapper. This could be replaced in the future with an
  // extension mechanism on the MlirContext for stashing user pointers.
  // Note that this holds a handle, which does not imply ownership.
  // Mappings will be removed when the context is destructed.
  using LiveContextMap = llvm::DenseMap<void *, PyMlirContext *>;
  static LiveContextMap &getLiveContexts();

  // Interns all live modules associated with this context. Modules tracked
  // in this map are valid. When a module is invalidated, it is removed
  // from this map, and while it still exists as an instance, any
  // attempt to access it will raise an error.
  using LiveModuleMap =
      llvm::DenseMap<const void *, std::pair<pybind11::handle, PyModule *>>;
  LiveModuleMap liveModules;

  // Interns all live operations associated with this context. Operations
  // tracked in this map are valid. When an operation is invalidated, it is
  // removed from this map, and while it still exists as an instance, any
  // attempt to access it will raise an error.
  using LiveOperationMap =
      llvm::DenseMap<void *, std::pair<pybind11::handle, PyOperation *>>;
  LiveOperationMap liveOperations;

  bool emitErrorDiagnostics = false;

  MlirContext context;
  friend class PyModule;
  friend class PyOperation;
};

/// Used in function arguments when None should resolve to the current context
/// manager set instance.
class DefaultingPyMlirContext
    : public Defaulting<DefaultingPyMlirContext, PyMlirContext> {
public:
  using Defaulting::Defaulting;
  static constexpr const char kTypeDescription[] = "mlir.ir.Context";
  static PyMlirContext &resolve();
};

/// Base class for all objects that directly or indirectly depend on an
/// MlirContext. The lifetime of the context will extend at least to the
/// lifetime of these instances.
/// Immutable objects that depend on a context extend this directly.
class BaseContextObject {
public:
  BaseContextObject(PyMlirContextRef ref) : contextRef(std::move(ref)) {
    assert(this->contextRef &&
           "context object constructed with null context ref");
  }

  /// Accesses the context reference.
  PyMlirContextRef &getContext() { return contextRef; }

private:
  PyMlirContextRef contextRef;
};

/// Wrapper around an MlirLocation.
class PyLocation : public BaseContextObject {
public:
  PyLocation(PyMlirContextRef contextRef, MlirLocation loc)
      : BaseContextObject(std::move(contextRef)), loc(loc) {}

  operator MlirLocation() const { return loc; }
  MlirLocation get() const { return loc; }

  /// Enter and exit the context manager.
  pybind11::object contextEnter();
  void contextExit(const pybind11::object &excType,
                   const pybind11::object &excVal,
                   const pybind11::object &excTb);

  /// Gets a capsule wrapping the void* within the MlirLocation.
  pybind11::object getCapsule();

  /// Creates a PyLocation from the MlirLocation wrapped by a capsule.
  /// Note that PyLocation instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirLocation
  /// is taken by calling this function.
  static PyLocation createFromCapsule(pybind11::object capsule);

private:
  MlirLocation loc;
};

/// Python class mirroring the C MlirDiagnostic struct. Note that these structs
/// are only valid for the duration of a diagnostic callback and attempting
/// to access them outside of that will raise an exception. This applies to
/// nested diagnostics (in the notes) as well.
class PyDiagnostic {
public:
  PyDiagnostic(MlirDiagnostic diagnostic) : diagnostic(diagnostic) {}
  void invalidate();
  bool isValid() { return valid; }
  MlirDiagnosticSeverity getSeverity();
  PyLocation getLocation();
  pybind11::str getMessage();
  pybind11::tuple getNotes();

  /// Materialized diagnostic information. This is safe to access outside the
  /// diagnostic callback.
  struct DiagnosticInfo {
    MlirDiagnosticSeverity severity;
    PyLocation location;
    std::string message;
    std::vector<DiagnosticInfo> notes;
  };
  DiagnosticInfo getInfo();

private:
  MlirDiagnostic diagnostic;

  void checkValid();
  /// If notes have been materialized from the diagnostic, then this will
  /// be populated with the corresponding objects (all castable to
  /// PyDiagnostic).
  std::optional<pybind11::tuple> materializedNotes;
  bool valid = true;
};

/// Represents a diagnostic handler attached to the context. The handler's
/// callback will be invoked with PyDiagnostic instances until the detach()
/// method is called or the context is destroyed. A diagnostic handler can be
/// the subject of a `with` block, which will detach it when the block exits.
///
/// Since diagnostic handlers can call back into Python code which can do
/// unsafe things (i.e. recursively emitting diagnostics, raising exceptions,
/// etc), this is generally not deemed to be a great user-level API. Users
/// should generally use some form of DiagnosticCollector. If the handler raises
/// any exceptions, they will just be emitted to stderr and dropped.
///
/// The unique usage of this class means that its lifetime management is
/// different from most other parts of the API. Instances are always created
/// in an attached state and can transition to a detached state by either:
///   a) The context being destroyed and unregistering all handlers.
///   b) An explicit call to detach().
/// The object may remain live from a Python perspective for an arbitrary time
/// after detachment, but there is nothing the user can do with it (since there
/// is no way to attach an existing handler object).
class PyDiagnosticHandler {
public:
  PyDiagnosticHandler(MlirContext context, pybind11::object callback);
  ~PyDiagnosticHandler();

  bool isAttached() { return registeredID.has_value(); }
  bool getHadError() { return hadError; }

  /// Detaches the handler. Does nothing if not attached.
  void detach();

  pybind11::object contextEnter() { return pybind11::cast(this); }
  void contextExit(const pybind11::object &excType,
                   const pybind11::object &excVal,
                   const pybind11::object &excTb) {
    detach();
  }

private:
  MlirContext context;
  pybind11::object callback;
  std::optional<MlirDiagnosticHandlerID> registeredID;
  bool hadError = false;
  friend class PyMlirContext;
};

/// RAII object that captures any error diagnostics emitted to the provided
/// context.
struct PyMlirContext::ErrorCapture {
  ErrorCapture(PyMlirContextRef ctx)
      : ctx(ctx), handlerID(mlirContextAttachDiagnosticHandler(
                      ctx->get(), handler, /*userData=*/this,
                      /*deleteUserData=*/nullptr)) {}
  ~ErrorCapture() {
    mlirContextDetachDiagnosticHandler(ctx->get(), handlerID);
    assert(errors.empty() && "unhandled captured errors");
  }

  std::vector<PyDiagnostic::DiagnosticInfo> take() {
    return std::move(errors);
  };

private:
  PyMlirContextRef ctx;
  MlirDiagnosticHandlerID handlerID;
  std::vector<PyDiagnostic::DiagnosticInfo> errors;

  static MlirLogicalResult handler(MlirDiagnostic diag, void *userData);
};

/// Wrapper around an MlirDialect. This is exported as `DialectDescriptor` in
/// order to differentiate it from the `Dialect` base class which is extended by
/// plugins which extend dialect functionality through extension python code.
/// This should be seen as the "low-level" object and `Dialect` as the
/// high-level, user facing object.
class PyDialectDescriptor : public BaseContextObject {
public:
  PyDialectDescriptor(PyMlirContextRef contextRef, MlirDialect dialect)
      : BaseContextObject(std::move(contextRef)), dialect(dialect) {}

  MlirDialect get() { return dialect; }

private:
  MlirDialect dialect;
};

/// User-level object for accessing dialects with dotted syntax such as:
///   ctx.dialect.std
class PyDialects : public BaseContextObject {
public:
  PyDialects(PyMlirContextRef contextRef)
      : BaseContextObject(std::move(contextRef)) {}

  MlirDialect getDialectForKey(const std::string &key, bool attrError);
};

/// User-level dialect object. For dialects that have a registered extension,
/// this will be the base class of the extension dialect type. For un-extended,
/// objects of this type will be returned directly.
class PyDialect {
public:
  PyDialect(pybind11::object descriptor) : descriptor(std::move(descriptor)) {}

  pybind11::object getDescriptor() { return descriptor; }

private:
  pybind11::object descriptor;
};

/// Wrapper around an MlirDialectRegistry.
/// Upon construction, the Python wrapper takes ownership of the
/// underlying MlirDialectRegistry.
class PyDialectRegistry {
public:
  PyDialectRegistry() : registry(mlirDialectRegistryCreate()) {}
  PyDialectRegistry(MlirDialectRegistry registry) : registry(registry) {}
  ~PyDialectRegistry() {
    if (!mlirDialectRegistryIsNull(registry))
      mlirDialectRegistryDestroy(registry);
  }
  PyDialectRegistry(PyDialectRegistry &) = delete;
  PyDialectRegistry(PyDialectRegistry &&other) : registry(other.registry) {
    other.registry = {nullptr};
  }

  operator MlirDialectRegistry() const { return registry; }
  MlirDialectRegistry get() const { return registry; }

  pybind11::object getCapsule();
  static PyDialectRegistry createFromCapsule(pybind11::object capsule);

private:
  MlirDialectRegistry registry;
};

/// Used in function arguments when None should resolve to the current context
/// manager set instance.
class DefaultingPyLocation
    : public Defaulting<DefaultingPyLocation, PyLocation> {
public:
  using Defaulting::Defaulting;
  static constexpr const char kTypeDescription[] = "mlir.ir.Location";
  static PyLocation &resolve();

  operator MlirLocation() const { return *get(); }
};

/// Wrapper around MlirModule.
/// This is the top-level, user-owned object that contains regions/ops/blocks.
class PyModule;
using PyModuleRef = PyObjectRef<PyModule>;
class PyModule : public BaseContextObject {
public:
  /// Returns a PyModule reference for the given MlirModule. This may return
  /// a pre-existing or new object.
  static PyModuleRef forModule(MlirModule module);
  PyModule(PyModule &) = delete;
  PyModule(PyMlirContext &&) = delete;
  ~PyModule();

  /// Gets the backing MlirModule.
  MlirModule get() { return module; }

  /// Gets a strong reference to this module.
  PyModuleRef getRef() {
    return PyModuleRef(this,
                       pybind11::reinterpret_borrow<pybind11::object>(handle));
  }

  /// Gets a capsule wrapping the void* within the MlirModule.
  /// Note that the module does not (yet) provide a corresponding factory for
  /// constructing from a capsule as that would require uniquing PyModule
  /// instances, which is not currently done.
  pybind11::object getCapsule();

  /// Creates a PyModule from the MlirModule wrapped by a capsule.
  /// Note that PyModule instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirModule
  /// is taken by calling this function.
  static pybind11::object createFromCapsule(pybind11::object capsule);

private:
  PyModule(PyMlirContextRef contextRef, MlirModule module);
  MlirModule module;
  pybind11::handle handle;
};

/// Base class for PyOperation and PyOpView which exposes the primary, user
/// visible methods for manipulating it.
class PyOperationBase {
public:
  virtual ~PyOperationBase() = default;
  /// Implements the bound 'print' method and helps with others.
  void print(pybind11::object fileObject, bool binary,
             std::optional<int64_t> largeElementsLimit, bool enableDebugInfo,
             bool prettyDebugInfo, bool printGenericOpForm, bool useLocalScope,
             bool assumeVerified);
  pybind11::object getAsm(bool binary,
                          std::optional<int64_t> largeElementsLimit,
                          bool enableDebugInfo, bool prettyDebugInfo,
                          bool printGenericOpForm, bool useLocalScope,
                          bool assumeVerified);

  // Implement the bound 'writeBytecode' method.
  void writeBytecode(const pybind11::object &fileObject,
                     std::optional<int64_t> bytecodeVersion);

  /// Moves the operation before or after the other operation.
  void moveAfter(PyOperationBase &other);
  void moveBefore(PyOperationBase &other);

  /// Verify the operation. Throws `MLIRError` if verification fails, and
  /// returns `true` otherwise.
  bool verify();

  /// Each must provide access to the raw Operation.
  virtual PyOperation &getOperation() = 0;
};

/// Wrapper around PyOperation.
/// Operations exist in either an attached (dependent) or detached (top-level)
/// state. In the detached state (as on creation), an operation is owned by
/// the creator and its lifetime extends either until its reference count
/// drops to zero or it is attached to a parent, at which point its lifetime
/// is bounded by its top-level parent reference.
class PyOperation;
using PyOperationRef = PyObjectRef<PyOperation>;
class PyOperation : public PyOperationBase, public BaseContextObject {
public:
  ~PyOperation() override;
  PyOperation &getOperation() override { return *this; }

  /// Returns a PyOperation for the given MlirOperation, optionally associating
  /// it with a parentKeepAlive.
  static PyOperationRef
  forOperation(PyMlirContextRef contextRef, MlirOperation operation,
               pybind11::object parentKeepAlive = pybind11::object());

  /// Creates a detached operation. The operation must not be associated with
  /// any existing live operation.
  static PyOperationRef
  createDetached(PyMlirContextRef contextRef, MlirOperation operation,
                 pybind11::object parentKeepAlive = pybind11::object());

  /// Parses a source string (either text assembly or bytecode), creating a
  /// detached operation.
  static PyOperationRef parse(PyMlirContextRef contextRef,
                              const std::string &sourceStr,
                              const std::string &sourceName);

  /// Detaches the operation from its parent block and updates its state
  /// accordingly.
  void detachFromParent() {
    mlirOperationRemoveFromParent(getOperation());
    setDetached();
    parentKeepAlive = pybind11::object();
  }

  /// Gets the backing operation.
  operator MlirOperation() const { return get(); }
  MlirOperation get() const {
    checkValid();
    return operation;
  }

  PyOperationRef getRef() {
    return PyOperationRef(
        this, pybind11::reinterpret_borrow<pybind11::object>(handle));
  }

  bool isAttached() { return attached; }
  void setAttached(const pybind11::object &parent = pybind11::object()) {
    assert(!attached && "operation already attached");
    attached = true;
  }
  void setDetached() {
    assert(attached && "operation already detached");
    attached = false;
  }
  void checkValid() const;

  /// Gets the owning block or raises an exception if the operation has no
  /// owning block.
  PyBlock getBlock();

  /// Gets the parent operation or raises an exception if the operation has
  /// no parent.
  std::optional<PyOperationRef> getParentOperation();

  /// Gets a capsule wrapping the void* within the MlirOperation.
  pybind11::object getCapsule();

  /// Creates a PyOperation from the MlirOperation wrapped by a capsule.
  /// Ownership of the underlying MlirOperation is taken by calling this
  /// function.
  static pybind11::object createFromCapsule(pybind11::object capsule);

  /// Creates an operation. See corresponding python docstring.
  static pybind11::object
  create(const std::string &name, std::optional<std::vector<PyType *>> results,
         std::optional<std::vector<PyValue *>> operands,
         std::optional<pybind11::dict> attributes,
         std::optional<std::vector<PyBlock *>> successors, int regions,
         DefaultingPyLocation location, const pybind11::object &ip,
         bool inferType);

  /// Creates an OpView suitable for this operation.
  pybind11::object createOpView();

  /// Erases the underlying MlirOperation, removes its pointer from the
  /// parent context's live operations map, and sets the valid bit false.
  void erase();

  /// Invalidate the operation.
  void setInvalid() { valid = false; }

  /// Clones this operation.
  pybind11::object clone(const pybind11::object &ip);

private:
  PyOperation(PyMlirContextRef contextRef, MlirOperation operation);
  static PyOperationRef createInstance(PyMlirContextRef contextRef,
                                       MlirOperation operation,
                                       pybind11::object parentKeepAlive);

  MlirOperation operation;
  pybind11::handle handle;
  // Keeps the parent alive, regardless of whether it is an Operation or
  // Module.
  // TODO: As implemented, this facility is only sufficient for modeling the
  // trivial module parent back-reference. Generalize this to also account for
  // transitions from detached to attached and address TODOs in the
  // ir_operation.py regarding testing corresponding lifetime guarantees.
  pybind11::object parentKeepAlive;
  bool attached = true;
  bool valid = true;

  friend class PyOperationBase;
  friend class PySymbolTable;
};

/// A PyOpView is equivalent to the C++ "Op" wrappers: these are the basis for
/// providing more instance-specific accessors and serve as the base class for
/// custom ODS-style operation classes. Since this class is subclass on the
/// python side, it must present an __init__ method that operates in pure
/// python types.
class PyOpView : public PyOperationBase {
public:
  PyOpView(const pybind11::object &operationObject);
  PyOperation &getOperation() override { return operation; }

  pybind11::object getOperationObject() { return operationObject; }

  static pybind11::object buildGeneric(
      const pybind11::object &cls, std::optional<pybind11::list> resultTypeList,
      pybind11::list operandList, std::optional<pybind11::dict> attributes,
      std::optional<std::vector<PyBlock *>> successors,
      std::optional<int> regions, DefaultingPyLocation location,
      const pybind11::object &maybeIp);

  /// Construct an instance of a class deriving from OpView, bypassing its
  /// `__init__` method. The derived class will typically define a constructor
  /// that provides a convenient builder, but we need to side-step this when
  /// constructing an `OpView` for an already-built operation.
  ///
  /// The caller is responsible for verifying that `operation` is a valid
  /// operation to construct `cls` with.
  static pybind11::object constructDerived(const pybind11::object &cls,
                                           const PyOperation &operation);

private:
  PyOperation &operation;           // For efficient, cast-free access from C++
  pybind11::object operationObject; // Holds the reference.
};

/// Wrapper around an MlirRegion.
/// Regions are managed completely by their containing operation. Unlike the
/// C++ API, the python API does not support detached regions.
class PyRegion {
public:
  PyRegion(PyOperationRef parentOperation, MlirRegion region)
      : parentOperation(std::move(parentOperation)), region(region) {
    assert(!mlirRegionIsNull(region) && "python region cannot be null");
  }
  operator MlirRegion() const { return region; }

  MlirRegion get() { return region; }
  PyOperationRef &getParentOperation() { return parentOperation; }

  void checkValid() { return parentOperation->checkValid(); }

private:
  PyOperationRef parentOperation;
  MlirRegion region;
};

/// Wrapper around an MlirAsmState.
class PyAsmState {
 public:
  PyAsmState(MlirValue value, bool useLocalScope) {
    flags = mlirOpPrintingFlagsCreate();
    // The OpPrintingFlags are not exposed Python side, create locally and
    // associate lifetime with the state.
    if (useLocalScope)
      mlirOpPrintingFlagsUseLocalScope(flags);
    state = mlirAsmStateCreateForValue(value, flags);
  }

  PyAsmState(PyOperationBase &operation, bool useLocalScope) {
    flags = mlirOpPrintingFlagsCreate();
    // The OpPrintingFlags are not exposed Python side, create locally and
    // associate lifetime with the state.
    if (useLocalScope)
      mlirOpPrintingFlagsUseLocalScope(flags);
    state =
        mlirAsmStateCreateForOperation(operation.getOperation().get(), flags);
  }
  ~PyAsmState() {
    mlirOpPrintingFlagsDestroy(flags);
  }
  // Delete copy constructors.
  PyAsmState(PyAsmState &other) = delete;
  PyAsmState(const PyAsmState &other) = delete;

  MlirAsmState get() { return state; }

 private:
  MlirAsmState state;
  MlirOpPrintingFlags flags;
};

/// Wrapper around an MlirBlock.
/// Blocks are managed completely by their containing operation. Unlike the
/// C++ API, the python API does not support detached blocks.
class PyBlock {
public:
  PyBlock(PyOperationRef parentOperation, MlirBlock block)
      : parentOperation(std::move(parentOperation)), block(block) {
    assert(!mlirBlockIsNull(block) && "python block cannot be null");
  }

  MlirBlock get() { return block; }
  PyOperationRef &getParentOperation() { return parentOperation; }

  void checkValid() { return parentOperation->checkValid(); }

  /// Gets a capsule wrapping the void* within the MlirBlock.
  pybind11::object getCapsule();

private:
  PyOperationRef parentOperation;
  MlirBlock block;
};

/// An insertion point maintains a pointer to a Block and a reference operation.
/// Calls to insert() will insert a new operation before the
/// reference operation. If the reference operation is null, then appends to
/// the end of the block.
class PyInsertionPoint {
public:
  /// Creates an insertion point positioned after the last operation in the
  /// block, but still inside the block.
  PyInsertionPoint(PyBlock &block);
  /// Creates an insertion point positioned before a reference operation.
  PyInsertionPoint(PyOperationBase &beforeOperationBase);

  /// Shortcut to create an insertion point at the beginning of the block.
  static PyInsertionPoint atBlockBegin(PyBlock &block);
  /// Shortcut to create an insertion point before the block terminator.
  static PyInsertionPoint atBlockTerminator(PyBlock &block);

  /// Inserts an operation.
  void insert(PyOperationBase &operationBase);

  /// Enter and exit the context manager.
  pybind11::object contextEnter();
  void contextExit(const pybind11::object &excType,
                   const pybind11::object &excVal,
                   const pybind11::object &excTb);

  PyBlock &getBlock() { return block; }
  std::optional<PyOperationRef> &getRefOperation() { return refOperation; }

private:
  // Trampoline constructor that avoids null initializing members while
  // looking up parents.
  PyInsertionPoint(PyBlock block, std::optional<PyOperationRef> refOperation)
      : refOperation(std::move(refOperation)), block(std::move(block)) {}

  std::optional<PyOperationRef> refOperation;
  PyBlock block;
};
/// Wrapper around the generic MlirType.
/// The lifetime of a type is bound by the PyContext that created it.
class PyType : public BaseContextObject {
public:
  PyType(PyMlirContextRef contextRef, MlirType type)
      : BaseContextObject(std::move(contextRef)), type(type) {}
  bool operator==(const PyType &other) const;
  operator MlirType() const { return type; }
  MlirType get() const { return type; }

  /// Gets a capsule wrapping the void* within the MlirType.
  pybind11::object getCapsule();

  /// Creates a PyType from the MlirType wrapped by a capsule.
  /// Note that PyType instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirType
  /// is taken by calling this function.
  static PyType createFromCapsule(pybind11::object capsule);

private:
  MlirType type;
};

/// A TypeID provides an efficient and unique identifier for a specific C++
/// type. This allows for a C++ type to be compared, hashed, and stored in an
/// opaque context. This class wraps around the generic MlirTypeID.
class PyTypeID {
public:
  PyTypeID(MlirTypeID typeID) : typeID(typeID) {}
  // Note, this tests whether the underlying TypeIDs are the same,
  // not whether the wrapper MlirTypeIDs are the same, nor whether
  // the PyTypeID objects are the same (i.e., PyTypeID is a value type).
  bool operator==(const PyTypeID &other) const;
  operator MlirTypeID() const { return typeID; }
  MlirTypeID get() { return typeID; }

  /// Gets a capsule wrapping the void* within the MlirTypeID.
  pybind11::object getCapsule();

  /// Creates a PyTypeID from the MlirTypeID wrapped by a capsule.
  static PyTypeID createFromCapsule(pybind11::object capsule);

private:
  MlirTypeID typeID;
};

/// CRTP base classes for Python types that subclass Type and should be
/// castable from it (i.e. via something like IntegerType(t)).
/// By default, type class hierarchies are one level deep (i.e. a
/// concrete type class extends PyType); however, intermediate python-visible
/// base classes can be modeled by specifying a BaseTy.
template <typename DerivedTy, typename BaseTy = PyType>
class PyConcreteType : public BaseTy {
public:
  // Derived classes must define statics for:
  //   IsAFunctionTy isaFunction
  //   const char *pyClassName
  using ClassTy = pybind11::class_<DerivedTy, BaseTy>;
  using IsAFunctionTy = bool (*)(MlirType);
  using GetTypeIDFunctionTy = MlirTypeID (*)();
  static constexpr GetTypeIDFunctionTy getTypeIdFunction = nullptr;

  PyConcreteType() = default;
  PyConcreteType(PyMlirContextRef contextRef, MlirType t)
      : BaseTy(std::move(contextRef), t) {}
  PyConcreteType(PyType &orig)
      : PyConcreteType(orig.getContext(), castFrom(orig)) {}

  static MlirType castFrom(PyType &orig) {
    if (!DerivedTy::isaFunction(orig)) {
      auto origRepr = pybind11::repr(pybind11::cast(orig)).cast<std::string>();
      throw py::value_error((llvm::Twine("Cannot cast type to ") +
                             DerivedTy::pyClassName + " (from " + origRepr +
                             ")")
                                .str());
    }
    return orig;
  }

  static void bind(pybind11::module &m) {
    auto cls = ClassTy(m, DerivedTy::pyClassName, pybind11::module_local());
    cls.def(pybind11::init<PyType &>(), pybind11::keep_alive<0, 1>(),
            pybind11::arg("cast_from_type"));
    cls.def_static(
        "isinstance",
        [](PyType &otherType) -> bool {
          return DerivedTy::isaFunction(otherType);
        },
        pybind11::arg("other"));
    cls.def_property_readonly_static(
        "static_typeid", [](py::object & /*class*/) -> MlirTypeID {
          if (DerivedTy::getTypeIdFunction)
            return DerivedTy::getTypeIdFunction();
          throw py::attribute_error(
              (DerivedTy::pyClassName + llvm::Twine(" has no typeid.")).str());
        });
    cls.def_property_readonly("typeid", [](PyType &self) {
      return py::cast(self).attr("typeid").cast<MlirTypeID>();
    });
    cls.def("__repr__", [](DerivedTy &self) {
      PyPrintAccumulator printAccum;
      printAccum.parts.append(DerivedTy::pyClassName);
      printAccum.parts.append("(");
      mlirTypePrint(self, printAccum.getCallback(), printAccum.getUserData());
      printAccum.parts.append(")");
      return printAccum.join();
    });

    if (DerivedTy::getTypeIdFunction) {
      PyGlobals::get().registerTypeCaster(
          DerivedTy::getTypeIdFunction(),
          pybind11::cpp_function(
              [](PyType pyType) -> DerivedTy { return pyType; }));
    }

    DerivedTy::bindDerived(cls);
  }

  /// Implemented by derived classes to add methods to the Python subclass.
  static void bindDerived(ClassTy &m) {}
};

/// Wrapper around the generic MlirAttribute.
/// The lifetime of a type is bound by the PyContext that created it.
class PyAttribute : public BaseContextObject {
public:
  PyAttribute(PyMlirContextRef contextRef, MlirAttribute attr)
      : BaseContextObject(std::move(contextRef)), attr(attr) {}
  bool operator==(const PyAttribute &other) const;
  operator MlirAttribute() const { return attr; }
  MlirAttribute get() const { return attr; }

  /// Gets a capsule wrapping the void* within the MlirAttribute.
  pybind11::object getCapsule();

  /// Creates a PyAttribute from the MlirAttribute wrapped by a capsule.
  /// Note that PyAttribute instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirAttribute
  /// is taken by calling this function.
  static PyAttribute createFromCapsule(pybind11::object capsule);

private:
  MlirAttribute attr;
};

/// Represents a Python MlirNamedAttr, carrying an optional owned name.
/// TODO: Refactor this and the C-API to be based on an Identifier owned
/// by the context so as to avoid ownership issues here.
class PyNamedAttribute {
public:
  /// Constructs a PyNamedAttr that retains an owned name. This should be
  /// used in any code that originates an MlirNamedAttribute from a python
  /// string.
  /// The lifetime of the PyNamedAttr must extend to the lifetime of the
  /// passed attribute.
  PyNamedAttribute(MlirAttribute attr, std::string ownedName);

  MlirNamedAttribute namedAttr;

private:
  // Since the MlirNamedAttr contains an internal pointer to the actual
  // memory of the owned string, it must be heap allocated to remain valid.
  // Otherwise, strings that fit within the small object optimization threshold
  // will have their memory address change as the containing object is moved,
  // resulting in an invalid aliased pointer.
  std::unique_ptr<std::string> ownedName;
};

/// CRTP base classes for Python attributes that subclass Attribute and should
/// be castable from it (i.e. via something like StringAttr(attr)).
/// By default, attribute class hierarchies are one level deep (i.e. a
/// concrete attribute class extends PyAttribute); however, intermediate
/// python-visible base classes can be modeled by specifying a BaseTy.
template <typename DerivedTy, typename BaseTy = PyAttribute>
class PyConcreteAttribute : public BaseTy {
public:
  // Derived classes must define statics for:
  //   IsAFunctionTy isaFunction
  //   const char *pyClassName
  using ClassTy = pybind11::class_<DerivedTy, BaseTy>;
  using IsAFunctionTy = bool (*)(MlirAttribute);
  using GetTypeIDFunctionTy = MlirTypeID (*)();
  static constexpr GetTypeIDFunctionTy getTypeIdFunction = nullptr;

  PyConcreteAttribute() = default;
  PyConcreteAttribute(PyMlirContextRef contextRef, MlirAttribute attr)
      : BaseTy(std::move(contextRef), attr) {}
  PyConcreteAttribute(PyAttribute &orig)
      : PyConcreteAttribute(orig.getContext(), castFrom(orig)) {}

  static MlirAttribute castFrom(PyAttribute &orig) {
    if (!DerivedTy::isaFunction(orig)) {
      auto origRepr = pybind11::repr(pybind11::cast(orig)).cast<std::string>();
      throw py::value_error((llvm::Twine("Cannot cast attribute to ") +
                             DerivedTy::pyClassName + " (from " + origRepr +
                             ")")
                                .str());
    }
    return orig;
  }

  static void bind(pybind11::module &m) {
    auto cls = ClassTy(m, DerivedTy::pyClassName, pybind11::buffer_protocol(),
                       pybind11::module_local());
    cls.def(pybind11::init<PyAttribute &>(), pybind11::keep_alive<0, 1>(),
            pybind11::arg("cast_from_attr"));
    cls.def_static(
        "isinstance",
        [](PyAttribute &otherAttr) -> bool {
          return DerivedTy::isaFunction(otherAttr);
        },
        pybind11::arg("other"));
    cls.def_property_readonly(
        "type", [](PyAttribute &attr) { return mlirAttributeGetType(attr); });
    cls.def_property_readonly_static(
        "static_typeid", [](py::object & /*class*/) -> MlirTypeID {
          if (DerivedTy::getTypeIdFunction)
            return DerivedTy::getTypeIdFunction();
          throw py::attribute_error(
              (DerivedTy::pyClassName + llvm::Twine(" has no typeid.")).str());
        });
    cls.def_property_readonly("typeid", [](PyAttribute &self) {
      return py::cast(self).attr("typeid").cast<MlirTypeID>();
    });
    cls.def("__repr__", [](DerivedTy &self) {
      PyPrintAccumulator printAccum;
      printAccum.parts.append(DerivedTy::pyClassName);
      printAccum.parts.append("(");
      mlirAttributePrint(self, printAccum.getCallback(),
                         printAccum.getUserData());
      printAccum.parts.append(")");
      return printAccum.join();
    });

    if (DerivedTy::getTypeIdFunction) {
      PyGlobals::get().registerTypeCaster(
          DerivedTy::getTypeIdFunction(),
          pybind11::cpp_function([](PyAttribute pyAttribute) -> DerivedTy {
            return pyAttribute;
          }));
    }

    DerivedTy::bindDerived(cls);
  }

  /// Implemented by derived classes to add methods to the Python subclass.
  static void bindDerived(ClassTy &m) {}
};

/// Wrapper around the generic MlirValue.
/// Values are managed completely by the operation that resulted in their
/// definition. For op result value, this is the operation that defines the
/// value. For block argument values, this is the operation that contains the
/// block to which the value is an argument (blocks cannot be detached in Python
/// bindings so such operation always exists).
class PyValue {
public:
  PyValue(PyOperationRef parentOperation, MlirValue value)
      : parentOperation(std::move(parentOperation)), value(value) {}
  operator MlirValue() const { return value; }

  MlirValue get() { return value; }
  PyOperationRef &getParentOperation() { return parentOperation; }

  void checkValid() { return parentOperation->checkValid(); }

  /// Gets a capsule wrapping the void* within the MlirValue.
  pybind11::object getCapsule();

  /// Creates a PyValue from the MlirValue wrapped by a capsule. Ownership of
  /// the underlying MlirValue is still tied to the owning operation.
  static PyValue createFromCapsule(pybind11::object capsule);

private:
  PyOperationRef parentOperation;
  MlirValue value;
};

/// Wrapper around MlirAffineExpr. Affine expressions are owned by the context.
class PyAffineExpr : public BaseContextObject {
public:
  PyAffineExpr(PyMlirContextRef contextRef, MlirAffineExpr affineExpr)
      : BaseContextObject(std::move(contextRef)), affineExpr(affineExpr) {}
  bool operator==(const PyAffineExpr &other) const;
  operator MlirAffineExpr() const { return affineExpr; }
  MlirAffineExpr get() const { return affineExpr; }

  /// Gets a capsule wrapping the void* within the MlirAffineExpr.
  pybind11::object getCapsule();

  /// Creates a PyAffineExpr from the MlirAffineExpr wrapped by a capsule.
  /// Note that PyAffineExpr instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirAffineExpr
  /// is taken by calling this function.
  static PyAffineExpr createFromCapsule(pybind11::object capsule);

  PyAffineExpr add(const PyAffineExpr &other) const;
  PyAffineExpr mul(const PyAffineExpr &other) const;
  PyAffineExpr floorDiv(const PyAffineExpr &other) const;
  PyAffineExpr ceilDiv(const PyAffineExpr &other) const;
  PyAffineExpr mod(const PyAffineExpr &other) const;

private:
  MlirAffineExpr affineExpr;
};

class PyAffineMap : public BaseContextObject {
public:
  PyAffineMap(PyMlirContextRef contextRef, MlirAffineMap affineMap)
      : BaseContextObject(std::move(contextRef)), affineMap(affineMap) {}
  bool operator==(const PyAffineMap &other) const;
  operator MlirAffineMap() const { return affineMap; }
  MlirAffineMap get() const { return affineMap; }

  /// Gets a capsule wrapping the void* within the MlirAffineMap.
  pybind11::object getCapsule();

  /// Creates a PyAffineMap from the MlirAffineMap wrapped by a capsule.
  /// Note that PyAffineMap instances are uniqued, so the returned object
  /// may be a pre-existing object. Ownership of the underlying MlirAffineMap
  /// is taken by calling this function.
  static PyAffineMap createFromCapsule(pybind11::object capsule);

private:
  MlirAffineMap affineMap;
};

class PyIntegerSet : public BaseContextObject {
public:
  PyIntegerSet(PyMlirContextRef contextRef, MlirIntegerSet integerSet)
      : BaseContextObject(std::move(contextRef)), integerSet(integerSet) {}
  bool operator==(const PyIntegerSet &other) const;
  operator MlirIntegerSet() const { return integerSet; }
  MlirIntegerSet get() const { return integerSet; }

  /// Gets a capsule wrapping the void* within the MlirIntegerSet.
  pybind11::object getCapsule();

  /// Creates a PyIntegerSet from the MlirAffineMap wrapped by a capsule.
  /// Note that PyIntegerSet instances may be uniqued, so the returned object
  /// may be a pre-existing object. Integer sets are owned by the context.
  static PyIntegerSet createFromCapsule(pybind11::object capsule);

private:
  MlirIntegerSet integerSet;
};

/// Bindings for MLIR symbol tables.
class PySymbolTable {
public:
  /// Constructs a symbol table for the given operation.
  explicit PySymbolTable(PyOperationBase &operation);

  /// Destroys the symbol table.
  ~PySymbolTable() { mlirSymbolTableDestroy(symbolTable); }

  /// Returns the symbol (opview) with the given name, throws if there is no
  /// such symbol in the table.
  pybind11::object dunderGetItem(const std::string &name);

  /// Removes the given operation from the symbol table and erases it.
  void erase(PyOperationBase &symbol);

  /// Removes the operation with the given name from the symbol table and erases
  /// it, throws if there is no such symbol in the table.
  void dunderDel(const std::string &name);

  /// Inserts the given operation into the symbol table. The operation must have
  /// the symbol trait.
  MlirAttribute insert(PyOperationBase &symbol);

  /// Gets and sets the name of a symbol op.
  static MlirAttribute getSymbolName(PyOperationBase &symbol);
  static void setSymbolName(PyOperationBase &symbol, const std::string &name);

  /// Gets and sets the visibility of a symbol op.
  static MlirAttribute getVisibility(PyOperationBase &symbol);
  static void setVisibility(PyOperationBase &symbol,
                            const std::string &visibility);

  /// Replaces all symbol uses within an operation. See the API
  /// mlirSymbolTableReplaceAllSymbolUses for all caveats.
  static void replaceAllSymbolUses(const std::string &oldSymbol,
                                   const std::string &newSymbol,
                                   PyOperationBase &from);

  /// Walks all symbol tables under and including 'from'.
  static void walkSymbolTables(PyOperationBase &from, bool allSymUsesVisible,
                               pybind11::object callback);

  /// Casts the bindings class into the C API structure.
  operator MlirSymbolTable() { return symbolTable; }

private:
  PyOperationRef operation;
  MlirSymbolTable symbolTable;
};

/// Custom exception that allows access to error diagnostic information. This is
/// converted to the `ir.MLIRError` python exception when thrown.
struct MLIRError {
  MLIRError(llvm::Twine message,
            std::vector<PyDiagnostic::DiagnosticInfo> &&errorDiagnostics = {})
      : message(message.str()), errorDiagnostics(std::move(errorDiagnostics)) {}
  std::string message;
  std::vector<PyDiagnostic::DiagnosticInfo> errorDiagnostics;
};

void populateIRAffine(pybind11::module &m);
void populateIRAttributes(pybind11::module &m);
void populateIRCore(pybind11::module &m);
void populateIRInterfaces(pybind11::module &m);
void populateIRTypes(pybind11::module &m);

} // namespace python
} // namespace mlir

namespace pybind11 {
namespace detail {

template <>
struct type_caster<mlir::python::DefaultingPyMlirContext>
    : MlirDefaultingCaster<mlir::python::DefaultingPyMlirContext> {};
template <>
struct type_caster<mlir::python::DefaultingPyLocation>
    : MlirDefaultingCaster<mlir::python::DefaultingPyLocation> {};

} // namespace detail
} // namespace pybind11

#endif // MLIR_BINDINGS_PYTHON_IRMODULES_H

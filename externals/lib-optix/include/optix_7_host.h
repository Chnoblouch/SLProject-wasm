/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

/// @file
/// @author NVIDIA Corporation
/// @brief  OptiX public API header
///
/// OptiX host include file -- includes the host api if compiling host code.
/// For the math library routines include optix_math.h

#if !defined( __OPTIX_INCLUDE_INTERNAL_HEADERS__ )
#error("optix_7_host.h is an internal header file and must not be used directly.  Please use optix_host.h or optix.h instead.")
#endif

#ifndef __optix_optix_7_host_h__
#define __optix_optix_7_host_h__

#include "optix_7_types.h"
#if !defined( OPTIX_DONT_INCLUDE_CUDA )
// If OPTIX_DONT_INCLUDE_CUDA is defined, cuda driver types must be defined through other
// means before including optix headers.
#include <cuda.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup optix_host_api Host API
/// \brief OptiX Host API

/// \defgroup optix_host_api_error_handling Error handling
/// \ingroup optix_host_api
//@{

/// Returns a string containing the name of an error code in the enum.
///
/// Output is a string representation of the enum.  For example "OPTIX_SUCCESS" for
/// OPTIX_SUCCESS and "OPTIX_ERROR_INVALID_VALUE" for OPTIX_ERROR_INVALID_VALUE.
///
/// If the error code is not recognized, "Unrecognized OptixResult code" is returned.
///
/// \param[in] result  OptixResult enum to generate string name for
///
/// \see #optixGetErrorString
const char* optixGetErrorName( OptixResult result );

/// Returns the description string for an error code.
///
/// Output is a string description of the enum.  For example "Success" for
/// OPTIX_SUCCESS and "Invalid value" for OPTIX_ERROR_INVALID_VALUE.
///
/// If the error code is not recognized, "Unrecognized OptixResult code" is returned.
///
/// \param[in] result  OptixResult enum to generate string description for
///
/// \see #optixGetErrorName
const char* optixGetErrorString( OptixResult result );

//@}
/// \defgroup optix_host_api_device_context Device context
/// \ingroup optix_host_api
//@{

/// Create a device context associated with the CUDA context specified with 'fromContext'.
///
/// If zero is specified for 'fromContext', OptiX will use the current CUDA context. The
/// CUDA context should be initialized before calling optixDeviceContextCreate.
///
/// \param[in] fromContext
/// \param[in] options
/// \param[out] context
/// \return
/// - OPTIX_ERROR_CUDA_NOT_INITIALIZED
///   If using zero for 'fromContext' and CUDA has not been initialized yet on the calling
///   thread.
/// - OPTIX_ERROR_CUDA_ERROR
///   CUDA operation failed.
/// - OPTIX_ERROR_HOST_OUT_OF_MEMORY
///   Heap allocation failed.
/// - OPTIX_ERROR_INTERNAL_ERROR
///   Internal error
OptixResult optixDeviceContextCreate( CUcontext fromContext, const OptixDeviceContextOptions* options, OptixDeviceContext* context );

/// Destroys all CPU and GPU state associated with the device.
///
/// It will attempt to block on CUDA streams that have launch work outstanding.
///
/// Any API objects, such as OptixModule and OptixPipeline, not already destroyed will be
/// destroyed.
///
/// Thread safety: A device context must not be destroyed while it is still in use by concurrent API calls in other threads.
OptixResult optixDeviceContextDestroy( OptixDeviceContext context );

/// Query properties of a device context.
///
/// \param[in] context     the device context to query the property for
/// \param[in] property    the property to query
/// \param[out] value      pointer to the returned
/// \param[in] sizeInBytes size of output
OptixResult optixDeviceContextGetProperty( OptixDeviceContext context, OptixDeviceProperty property, void* value, size_t sizeInBytes );

/// Sets the current log callback method.
///
/// See #OptixLogCallback for more details.
///
/// Thread safety: It is guaranteed that the callback itself (callbackFunction and callbackData) are updated atomically.
/// It is not guaranteed that the callback itself (callbackFunction and callbackData) and the callbackLevel are updated
/// atomically. It is unspecified when concurrent API calls using the same context start to make use of the new
/// callback method.
///
/// \param[in] context          the device context
/// \param[in] callbackFunction the callback function to call
/// \param[in] callbackData     pointer to data passed to callback function while invoking it
/// \param[in] callbackLevel    callback level
OptixResult optixDeviceContextSetLogCallback( OptixDeviceContext context,
                                              OptixLogCallback   callbackFunction,
                                              void*              callbackData,
                                              unsigned int       callbackLevel );

/// Enables or disables the disk cache.
///
/// If caching was previously disabled, enabling it will attempt to initialize
/// the disk cache database using the currently configured cache location. An
/// error will be returned if initialization fails.
///
/// Note that no in-memory cache is used, so no caching behavior will be observed if the disk cache
/// is disabled.
///
/// \param[in] context the device context
/// \param[in] enabled 1 to enabled, 0 to disable
OptixResult optixDeviceContextSetCacheEnabled( OptixDeviceContext context,
                                               int                enabled );

/// Sets the location of the disk cache.
///
/// The location is specified by a directory. This directory should not be used for other purposes
/// and will be created if it does not exist. An error will be returned if is not possible to
/// create the disk cache at the specified location for any reason (e.g., the path is invalid or
/// the directory is not writable). Caching will be disabled if the disk cache cannot be
/// initialized in the new location. If caching is disabled, no error will be returned until caching
/// is enabled. If the disk cache is located on a network file share, behavior is undefined.
///
/// The location of the disk cache can be overridden with the environment variable OPTIX_CACHE_PATH.
/// The environment variable takes precedence over this setting.
///
/// The default location depends on the operating system:
/// - Windows: %LOCALAPPDATA%\\NVIDIA\\OptixCache
/// - Linux:   /var/tmp/OptixCache_\<username\> (or /tmp/OptixCache_\<username\> if the first choice is not usable),
///            the underscore and username suffix are omitted if the username cannot be obtained
/// - MacOS X: /Library/Application Support/NVIDIA/OptixCache
///
/// \param[in] context  the device context
/// \param[in] location directory of disk cache
OptixResult optixDeviceContextSetCacheLocation( OptixDeviceContext context, const char* location );

/// Sets the low and high water marks for disk cache garbage collection.
///
/// Garbage collection is triggered when a new entry is written to the cache and
/// the current cache data size plus the size of the cache entry that is about
/// to be inserted exceeds the high water mark. Garbage collection proceeds until
/// the size reaches the low water mark. Garbage collection will always free enough
/// space to insert the new entry without exceeding the low water mark. Setting
/// either limit to zero will disable garbage collection. An error will be returned
/// if both limits are non-zero and the high water mark is smaller than the low water mark.
///
/// Note that garbage collection is performed only on writes to the disk cache. No garbage
/// collection is triggered on disk cache initialization or immediately when calling this function,
/// but on subsequent inserting of data into the database.
///
/// If the size of a compiled module exceeds the value configured for the high water
/// mark and garbage collection is enabled, the module will not be added to the cache
/// and a warning will be added to the log.
///
/// \param[in] context       the device context
/// \param[in] lowWaterMark  the low water mark
/// \param[in] highWaterMark the high water mark
OptixResult optixDeviceContextSetCacheDatabaseSizes( OptixDeviceContext context, size_t lowWaterMark, size_t highWaterMark );

/// Indicates whether the disk cache is enabled or disabled.
///
/// \param[in] context   the device context
/// \param[out] enabled  1 if enabled, 0 if disabled
OptixResult optixDeviceContextGetCacheEnabled( OptixDeviceContext context, int* enabled );
/// Returns the location of the disk cache.
///
/// \param[in] context      the device context
/// \param[out] location    directory of disk cache, null terminated if locationSize > 0
/// \param[in] locationSize locationSize
OptixResult optixDeviceContextGetCacheLocation( OptixDeviceContext context, char* location, size_t locationSize );

/// Returns the low and high water marks for disk cache garbage collection.
///
/// \param[in] context        the device context
/// \param[out] lowWaterMark  the low water mark
/// \param[out] highWaterMark the high water mark
OptixResult optixDeviceContextGetCacheDatabaseSizes( OptixDeviceContext context, size_t* lowWaterMark, size_t* highWaterMark );

//@}
/// \defgroup optix_host_api_pipelines Pipelines
/// \ingroup optix_host_api
//@{

/// logString is an optional buffer that contains compiler feedback and errors.  This
/// information is also passed to the context logger (if enabled), however it may be
/// difficult to correlate output to the logger to specific API invocations when using
/// multiple threads.  The output to logString will only contain feedback for this specific
/// invocation of this API call.
///
/// logStringSize as input should be a pointer to the number of bytes backing logString.
/// Upon return it contains the length of the log message (including the null terminator)
/// which may be greater than the input value.  In this case, the log message will be
/// truncated to fit into logString.
///
/// If logString or logStringSize are NULL, no output is written to logString.  If
/// logStringSize points to a value that is zero, no output is written.  This does not
/// affect output to the context logger if enabled.
///
/// \param[in] context
/// \param[in] pipelineCompileOptions
/// \param[in] pipelineLinkOptions
/// \param[in] programGroups          array of ProgramGroup objects
/// \param[in] numProgramGroups       number of ProgramGroup objects
/// \param[out] logString             Information will be written to this string. If logStringSize > 0 logString will be null terminated.
/// \param[in,out] logStringSize
/// \param[out] pipeline
OptixResult optixPipelineCreate( OptixDeviceContext                 context,
                                 const OptixPipelineCompileOptions* pipelineCompileOptions,
                                 const OptixPipelineLinkOptions*    pipelineLinkOptions,
                                 const OptixProgramGroup*           programGroups,
                                 unsigned int                       numProgramGroups,
                                 char*                              logString,
                                 size_t*                            logStringSize,
                                 OptixPipeline*                     pipeline );

/// Thread safety: A pipeline must not be destroyed while it is still in use by concurrent API calls in other threads.
OptixResult optixPipelineDestroy( OptixPipeline pipeline );

/// Sets the stack sizes for a pipeline.
///
/// Users are encouraged to see the programming guide and the implementations of the helper functions
/// to understand how to construct the stack sizes based on their particular needs.
///
/// If this method is not used, an internal default implementation is used. The default implementation is correct (but
/// not necessarily optimal) as long as the maximum depth of call trees of CC and DC programs is at most 2 and no motion transforms are used.
///
/// The maxTraversableGraphDepth responds to the maximal number of traversables visited when calling trace.
/// Every acceleration structure and motion transform count as one level of traversal.
/// E.g., for a simple IAS (instance acceleration structure) -> GAS (geometry acceleration structure)
/// traversal graph, the maxTraversableGraphDepth is two.
/// For IAS -> MT (motion transform) -> GAS, the maxTraversableGraphDepth is three.
/// Note that it does not matter whether a IAS or GAS has motion or not, it always counts as one.
/// Launching optix with exceptions turned on (see #OPTIX_EXCEPTION_FLAG_TRACE_DEPTH) will throw an exception
/// if the specified maxTraversableGraphDepth is too small.
///
/// \param[in] pipeline                             The pipeline to configure the stack size for.
/// \param[in] directCallableStackSizeFromTraversal The direct stack size requirement for direct callables invoked from IS or AH.
/// \param[in] directCallableStackSizeFromState     The direct stack size requirement for direct callables invoked from RG, MS, or CH.
/// \param[in] continuationStackSize                The continuation stack requirement.
/// \param[in] maxTraversableGraphDepth             The maximum depth of a traversable graph passed to trace.
OptixResult optixPipelineSetStackSize( OptixPipeline pipeline,
                                       unsigned int  directCallableStackSizeFromTraversal,
                                       unsigned int  directCallableStackSizeFromState,
                                       unsigned int  continuationStackSize,
                                       unsigned int  maxTraversableGraphDepth );

//@}
/// \defgroup optix_host_api_modules Modules
/// \ingroup optix_host_api
//@{

/// logString is an optional buffer that contains compiler feedback and errors.  This
/// information is also passed to the context logger (if enabled), however it may be
/// difficult to correlate output to the logger to specific API invocations when using
/// multiple threads.  The output to logString will only contain feedback for this specific
/// invocation of this API call.
///
/// logStringSize as input should be a pointer to the number of bytes backing logString.
/// Upon return it contains the length of the log message (including the null terminator)
/// which may be greater than the input value.  In this case, the log message will be
/// truncated to fit into logString.
///
/// If logString or logStringSize are NULL, no output is written to logString.  If
/// logStringSize points to a value that is zero, no output is written.  This does not
/// affect output to the context logger if enabled.
///
/// \param[in] context
/// \param[in] moduleCompileOptions
/// \param[in] pipelineCompileOptions All modules in a pipeline need to use the same values for the pipeline compile options.
/// \param[in] PTX
/// \param[in] PTXsize
/// \param[out] logString             Information will be written to this string. If logStringSize > 0 logString will be null terminated.
/// \param[in,out] logStringSize
/// \param[out] module
///
/// \return OPTIX_ERROR_INVALID_VALUE - context is 0, moduleCompileOptions is 0, pipelineCompileOptions is 0, PTX is 0, module is 0.
OptixResult optixModuleCreateFromPTX( OptixDeviceContext                 context,
                                      const OptixModuleCompileOptions*   moduleCompileOptions,
                                      const OptixPipelineCompileOptions* pipelineCompileOptions,
                                      const char*                        PTX,
                                      size_t                             PTXsize,
                                      char*                              logString,
                                      size_t*                            logStringSize,
                                      OptixModule*                       module );

/// Call for OptixModule objects created with optixModuleCreateFromPTX and optixModuleDeserialize.
///
/// Modules must not be destroyed while they are still used by any program group.
///
/// Thread safety: A module must not be destroyed while it is still in use by concurrent API calls in other threads.
OptixResult optixModuleDestroy( OptixModule module );

//@}
/// \defgroup optix_host_api_program_groups Program groups
/// \ingroup optix_host_api
//@{

/// Returns the stack sizes for the given program group.
///
/// \param[in] programGroup the program group
/// \param[out] stackSizes  the corresponding stack sizes
OptixResult optixProgramGroupGetStackSize( OptixProgramGroup programGroup, OptixStackSizes* stackSizes );

/// logString is an optional buffer that contains compiler feedback and errors.  This
/// information is also passed to the context logger (if enabled), however it may be
/// difficult to correlate output to the logger to specific API invocations when using
/// multiple threads.  The output to logString will only contain feedback for this specific
/// invocation of this API call.
///
/// logStringSize as input should be a pointer to the number of bytes backing logString.
/// Upon return it contains the length of the log message (including the null terminator)
/// which may be greater than the input value.  In this case, the log message will be
/// truncated to fit into logString.
///
/// If logString or logStringSize are NULL, no output is written to logString.  If
/// logStringSize points to a value that is zero, no output is written.  This does not
/// affect output to the context logger if enabled.
///
/// Creates numProgramGroups OptiXProgramGroup objects from the specified
/// OptixProgramGroupDesc array.  The size of the arrays must match.
///
/// \param[in] context
/// \param[in] programDescriptions    N * OptixProgramGroupDesc
/// \param[in] numProgramGroups       N
/// \param[in] options
/// \param[out] logString             Information will be written to this string. If logStringSize > 0 logString will be null terminated.
/// \param[in,out] logStringSize
/// \param[out] programGroups
OptixResult optixProgramGroupCreate( OptixDeviceContext              context,
                                     const OptixProgramGroupDesc*    programDescriptions,
                                     unsigned int                    numProgramGroups,
                                     const OptixProgramGroupOptions* options,
                                     char*                           logString,
                                     size_t*                         logStringSize,
                                     OptixProgramGroup*              programGroups );

/// Thread safety: A program group must not be destroyed while it is still in use by concurrent API calls in other threads.
OptixResult optixProgramGroupDestroy( OptixProgramGroup programGroup );

//@}
/// \defgroup optix_host_api_launches Launches
/// \ingroup optix_host_api
//@{

/// Where the magic happens.
///
/// The stream and pipeline must belong to the same device context.  Multiple launches
/// may be issues in parallel from multiple threads to different streams.
///
/// pipelineParamsSize number of bytes are copied from the device memory pointed to by
/// pipelineParams before launch.  It is an error if pipelineParamsSize is greater than the
/// size of the variable declared in modules and identified by
/// OptixPipelineCompileOptions::pipelineLaunchParamsVariableName. If the launch params
/// variable was optimized out or not found in the modules linked to the pipeline then
/// the pipelineParams and pipelineParamsSize parameters are ignored.
///
/// sbt points to the shader binding table, which defines shader
/// groupings and their resources. See the SBT spec.
///
/// \param[in] pipeline
/// \param[in] stream
/// \param[in] pipelineParams
/// \param[in] pipelineParamsSize
/// \param[in] sbt
/// \param[in] width              number of elements to compute
/// \param[in] height             number of elements to compute
/// \param[in] depth              number of elements to compute
///
/// Thread safety: In the current implementation concurrent launches to the same pipeline are not
/// supported.  Concurrent launches require separate OptixPipeline objects.
OptixResult optixLaunch( OptixPipeline                  pipeline,
                         CUstream                       stream,
                         CUdeviceptr                    pipelineParams,
                         size_t                         pipelineParamsSize,
                         const OptixShaderBindingTable* sbt,
                         unsigned int                   width,
                         unsigned int                   height,
                         unsigned int                   depth );

/// \param[in]  programGroup               the program group containing the program(s)
/// \param[out] sbtRecordHeaderHostPointer  the result sbt record header
OptixResult optixSbtRecordPackHeader( OptixProgramGroup programGroup, void* sbtRecordHeaderHostPointer );

//@}
/// \defgroup optix_host_api_acceleration_structures Acceleration structures
/// \ingroup optix_host_api
//@{

/// \param[in] context        device context of the pipeline
/// \param[in] accelOptions   accel options
/// \param[in] buildInputs    an array of OptixBuildInput objects
/// \param[in] numBuildInputs number of elements in buildInputs (must be at least 1)
/// \param[out] bufferSizes   fills in buffer sizes
OptixResult optixAccelComputeMemoryUsage( OptixDeviceContext            context,
                                          const OptixAccelBuildOptions* accelOptions,
                                          const OptixBuildInput*        buildInputs,
                                          unsigned int                  numBuildInputs,
                                          OptixAccelBufferSizes*        bufferSizes );

/// \param[in] context
/// \param[in] stream
/// \param[in] accelOptions             accel options
/// \param[in] buildInputs              an array of OptixBuildInput objects
/// \param[in] numBuildInputs           must be >= 1 for GAS, and == 1 for IAS
/// \param[in] tempBuffer               must be a multiple of OPTIX_ACCEL_BUFFER_BYTE_ALIGNMENT
/// \param[in] tempBufferSizeInBytes
/// \param[in] outputBuffer             must be a multiple of OPTIX_ACCEL_BUFFER_BYTE_ALIGNMENT
/// \param[in] outputBufferSizeInBytes
/// \param[out] outputHandle
/// \param[out] emittedProperties        types of requested properties and output buffers
///                                      OPTIX_PROPERTY_TYPE_AABBS is currently not supported
///                                      and must be requested via optixAccelEmitProperties
/// \param[in] numEmittedProperties      number of post-build properties to populate (may be zero)
OptixResult optixAccelBuild( OptixDeviceContext            context,
                             CUstream                      stream,
                             const OptixAccelBuildOptions* accelOptions,
                             const OptixBuildInput*        buildInputs,
                             unsigned int                  numBuildInputs,
                             CUdeviceptr                   tempBuffer,
                             size_t                        tempBufferSizeInBytes,
                             CUdeviceptr                   outputBuffer,
                             size_t                        outputBufferSizeInBytes,
                             OptixTraversableHandle*       outputHandle,
                             const OptixAccelEmitDesc*     emittedProperties,
                             unsigned int                  numEmittedProperties );

/// Obtain relocation information, stored in OptixAccelRelocationInfo, for a given context
/// and acceleration structure's traversable handle.
///
/// The relocation information can be passed to optixAccelCheckRelocationCompatibility to
/// determine if an acceleration structure, referenced by 'handle', can be relocated to a
/// different device's memory space (see #optixAccelCheckRelocationCompatibility).
///
/// When used with optixAccelRelocate, it provides data necessary for doing the relocation.
///
/// If the acceleration structure data associated with 'handle' is copied multiple times,
/// the same OptixAccelRelocationInfo can also be used on all copies.
///
/// \param[in] context
/// \param[in] handle
/// \param[out] info
/// \return OPTIX_ERROR_INVALID_VALUE will be returned for traversable handles that are not from
/// acceleration structure builds.
OptixResult optixAccelGetRelocationInfo( OptixDeviceContext context, OptixTraversableHandle handle, OptixAccelRelocationInfo* info );

/// Checks if an acceleration structure built using another OptixDeviceContext (that was
/// used to fill in 'info') is compatible with the OptixDeviceContext specified in the
/// 'context' parameter.
///
/// Any device is always compatible with itself.
///
/// \param[in] context
/// \param[in] info
/// \param[out] compatible If OPTIX_SUCCESS is returned 'compatible' will have the value of either:
/// - 0: This context is not compatible with acceleration structure data associated with 'info'.
/// - 1: This context is compatible.
OptixResult optixAccelCheckRelocationCompatibility( OptixDeviceContext context, const OptixAccelRelocationInfo* info, int* compatible );

/// optixAccelRelocate is called to update the acceleration structure after it has been
/// relocated.  Relocation is necessary when the acceleration structure's location in device
/// memory has changed.  optixAccelRelocate does not copy the memory.  This function only
/// operates on the relocated memory who's new location is specified by 'targetAccel'.
/// optixAccelRelocate also returns the new OptixTraversableHandle associated with
/// 'targetAccel'.  The original memory (source) is not required to be valid, only the
/// OptixAccelRelocationInfo.
///
/// Before copying the data and calling optixAccelRelocate,
/// optixAccelCheckRelocationCompatibility should be called to ensure the copy will be
/// compatible with the destination device context.
///
/// The memory pointed to by 'targetAccel' should be allocated with the same size as the
/// source acceleration.  Similar to the 'outputBuffer' used in optixAccelBuild, this
/// pointer must be a multiple of OPTIX_ACCEL_BUFFER_BYTE_ALIGNMENT.
///
/// The memory in 'targetAccel' must be allocated as long as the accel is in use.
///
/// When relocating an accel that contains instances, 'instanceTraversableHandles' and
/// 'numInstanceTraversableHandles' should be supplied.  These are the traversable handles
/// of the instances.  These can be used when also relocating the instances.  No updates to
/// the bounds are performed.  Use optixAccelBuild to update the bounds.
/// 'instanceTraversableHandles' and 'numInstanceTraversableHandles' may be zero when
/// relocating bottom level accel (i.e. an accel with no instances).
///
/// \param[in] context
/// \param[in] stream
/// \param[in] info
/// \param[in] instanceTraversableHandles
/// \param[in] numInstanceTraversableHandles
/// \param[in] targetAccel
/// \param[in] targetAccelSizeInBytes
/// \param[out] targetHandle
OptixResult optixAccelRelocate( OptixDeviceContext              context,
                                CUstream                        stream,
                                const OptixAccelRelocationInfo* info,
                                CUdeviceptr                     instanceTraversableHandles,
                                size_t                          numInstanceTraversableHandles,
                                CUdeviceptr                     targetAccel,
                                size_t                          targetAccelSizeInBytes,
                                OptixTraversableHandle*         targetHandle );

/// After building an acceleration structure, it can be copied in a compacted form to reduce
/// memory.  In order to be compacted, OPTIX_BUILD_FLAG_ALLOW_COMPACTION must be supplied in
/// OptixAccelBuildOptions::buildFlags passed to optixAccelBuild.
///
/// 'outputBuffer' is the pointer to where the compacted acceleration structure will be
/// written.  This pointer must be a multiple of OPTIX_ACCEL_BUFFER_BYTE_ALIGNMENT.
///
/// The size of the memory specified in 'outputBufferSizeInBytes' should be at least the
/// value computed using the OPTIX_PROPERTY_TYPE_COMPACTED_SIZE that was reported during
/// optixAccelBuild.
///
/// \param[in] context
/// \param[in] stream
/// \param[in] inputHandle
/// \param[in] outputBuffer
/// \param[in] outputBufferSizeInBytes
/// \param[out] outputHandle
OptixResult optixAccelCompact( OptixDeviceContext      context,
                               CUstream                stream,
                               OptixTraversableHandle  inputHandle,
                               CUdeviceptr             outputBuffer,
                               size_t                  outputBufferSizeInBytes,
                               OptixTraversableHandle* outputHandle );

/// \param[in] onDevice
/// \param[in] pointer            pointer to traversalbe allocated in OptixDeviceContext. This pointer must be a multiple of OPTIX_TRANSFORM_BYTE_ALIGNMENT
/// \param[in] traversableType    Type of OptixTraversableHandle to create
/// \param[out] traversableHandle traversable handle. traversableHandle must be in host memory
OptixResult optixConvertPointerToTraversableHandle( OptixDeviceContext      onDevice,
                                                    CUdeviceptr             pointer,
                                                    OptixTraversableType    traversableType,
                                                    OptixTraversableHandle* traversableHandle );

//@}
/// \defgroup optix_host_api_denoiser Denoiser
/// \ingroup optix_host_api
//@{

/// Creates a denoiser object with the given options.
///
/// \param[in] context
/// \param[in] options
/// \param[out] denoiser
OptixResult optixDenoiserCreate( OptixDeviceContext context, const OptixDenoiserOptions* options, OptixDenoiser* denoiser );

/// Sets the model of the denoiser.
///
/// If the kind is OPTIX_DENOISER_MODEL_KIND_USER, then the data and sizeInByes must not be
/// null and zero respectively.  For other kinds, these parameters must be zero.
///
/// \param[in] denoiser
/// \param[in] kind
/// \param[in] data
/// \param[in] sizeInBytes
OptixResult optixDenoiserSetModel( OptixDenoiser denoiser, OptixDenoiserModelKind kind, void* data, size_t sizeInBytes );

/// Destroys the denoiser object and any associated host resources.
OptixResult optixDenoiserDestroy( OptixDenoiser denoiser );

/// Computes the GPU memory resources required to execute the denoiser.
///
/// \param[in] denoiser
/// \param[in] outputWidth
/// \param[in] outputHeight
/// \param[out] returnSizes
OptixResult optixDenoiserComputeMemoryResources( const OptixDenoiser denoiser,
                                                 unsigned int        outputWidth,
                                                 unsigned int        outputHeight,
                                                 OptixDenoiserSizes* returnSizes );

/// Initializes the state required by the denoiser.
///
/// \param[in] denoiser
/// \param[in] stream
/// \param[in] outputWidth
/// \param[in] outputHeight
/// \param[in] denoiserState
/// \param[in] denoiserStateSizeInBytes
/// \param[in] scratch
/// \param[in] scratchSizeInBytes
OptixResult optixDenoiserSetup( OptixDenoiser denoiser,
                                CUstream      stream,
                                unsigned int  outputWidth,
                                unsigned int  outputHeight,
                                CUdeviceptr   denoiserState,
                                size_t        denoiserStateSizeInBytes,
                                CUdeviceptr   scratch,
                                size_t        scratchSizeInBytes );

/// Invokes denoiser on a set of input data and produces one output
/// image. Scratch memory must be available during the execution of the
/// denoiser.
///
/// \param[in] denoiser
/// \param[in] stream
/// \param[in] params
/// \param[in] denoiserState
/// \param[in] denoiserStateSizeInBytes
/// \param[in] inputLayers
/// \param[in] numInputLayers
/// \param[in] inputOffsetX
/// \param[in] inputOffsetY
/// \param[in] outputLayer
/// \param[in] scratch
/// \param[in] scratchSizeInBytes
OptixResult optixDenoiserInvoke( OptixDenoiser              denoiser,
                                 CUstream                   stream,
                                 const OptixDenoiserParams* params,
                                 CUdeviceptr                denoiserState,
                                 size_t                     denoiserStateSizeInBytes,
                                 const OptixImage2D*        inputLayers,
                                 unsigned int               numInputLayers,
                                 unsigned int               inputOffsetX,
                                 unsigned int               inputOffsetY,
                                 const OptixImage2D*        outputLayer,
                                 CUdeviceptr                scratch,
                                 size_t                     scratchSizeInBytes );

/// \param[in] denoiser
/// \param[in] stream
/// \param[in] inputImage
/// \param[out] outputIntensity    single float
/// \param[in] scratch
/// \param[in] scratchSizeInBytes
OptixResult optixDenoiserComputeIntensity( OptixDenoiser       denoiser,
                                           CUstream            stream,
                                           const OptixImage2D* inputImage,
                                           CUdeviceptr         outputIntensity,
                                           CUdeviceptr         scratch,
                                           size_t              scratchSizeInBytes );

//@}

#ifdef __cplusplus
}
#endif

#include "optix_function_table.h"

#endif  // __optix_optix_7_host_h__

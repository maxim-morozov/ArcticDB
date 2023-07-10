/* Copyright 2023 Man Group Operations Limited
 *
 * Use of this software is governed by the Business Source License 1.1 included in the file licenses/BSL.txt.
 *
 * As of the Change Date specified in that file, in accordance with the Business Source License, use of this software will be governed by the Apache License, version 2.0.
 */

#pragma once

#include <arcticdb/storage/storage.hpp>
#include <arcticdb/storage/storage_factory.hpp>
#include <arcticdb/storage/object_store_utils.hpp>
#include <arcticdb/log/log.hpp>
#include <arcticdb/entity/protobufs.hpp>
#include <arcticdb/util/composite.hpp>
#include <arcticdb/util/configs_map.hpp>
#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace arcticdb::storage::azure {

class AzureStorage final : public Storage<AzureStorage> {

    using Parent = Storage<AzureStorage>;
    friend Parent;

  public:
    // friend class AzureTestClientAccessor<AzureStorage>;
    using Config = arcticdb::proto::azure_storage::Config;

    AzureStorage(const LibraryPath &lib, OpenMode mode, const Config &conf);

    /**
     * Full object path in Azure bucket.
     */
    std::string get_key_path(const VariantKey& key) const;

  protected:
    void do_write(Composite<KeySegmentPair>&& kvs);

    void do_update(Composite<KeySegmentPair>&& kvs, UpdateOpts opts);

    template<class Visitor>
    void do_read(Composite<VariantKey>&& ks, Visitor &&visitor, ReadKeyOpts opts);

    void do_remove(Composite<VariantKey>&& ks, RemoveOpts opts);

    template<class Visitor>
    void do_iterate_type(KeyType key_type, Visitor &&visitor, const std::string &prefix);

    bool do_key_exists(const VariantKey& key);

    bool do_supports_prefix_matching() {
        return true;
    }

    bool do_fast_delete() {
        return false;
    }

  private:
    Azure::Storage::Blobs::BlobContainerClient container_client_;
    std::string root_folder_;
    unsigned int request_timeout_;
    Azure::Storage::Blobs::UploadBlockBlobFromOptions upload_option_;
    Azure::Storage::Blobs::DownloadBlobToOptions download_option_;

    Azure::Storage::Blobs::BlobClientOptions get_client_options(const Config &conf);
};


class AzureStorageFactory final : public StorageFactory<AzureStorageFactory> {
    using Parent = StorageFactory<AzureStorageFactory>;
    friend Parent;

  public:
    using Config = arcticdb::proto::azure_storage::Config;
    using StorageType = AzureStorage;

    AzureStorageFactory(const Config &conf) :
        conf_(conf) {
    }
  private:
    auto do_create_storage(const LibraryPath &lib, OpenMode mode) {
            return AzureStorage(lib, mode, conf_);
    }

    Config conf_;
};

inline arcticdb::proto::storage::VariantStorage pack_config(const std::string &container_name) {
    arcticdb::proto::storage::VariantStorage output;
    arcticdb::proto::azure_storage::Config cfg;
    cfg.set_container_name(container_name);
    util::pack_to_any(cfg, *output.mutable_config());
    return output;
}

inline arcticdb::proto::storage::VariantStorage pack_config(
        const std::string &container_name,
        const std::string &endpoint
        ) {
    arcticdb::proto::storage::VariantStorage output;
    arcticdb::proto::azure_storage::Config cfg;
    cfg.set_container_name(container_name);
    cfg.set_endpoint(endpoint);
    util::pack_to_any(cfg, *output.mutable_config());
    return output;
}

template<typename ConfigType>
std::shared_ptr<Azure::Storage::StorageSharedKeyCredential> get_azure_credentials(const ConfigType& conf) {
    return std::make_shared<Azure::Storage::StorageSharedKeyCredential>(conf.credential_name(), conf.credential_key());
}

} //namespace arcticdb::azure

#define ARCTICDB_AZURE_STORAGE_H_
#include <arcticdb/storage/azure/azure_storage-inl.hpp>
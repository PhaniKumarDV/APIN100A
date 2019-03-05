# ===============================================================================
#
#  Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================
import platform
import os

from sectools.common.crypto import cert, utils
from sectools.common.rpc import BinString
from sectools.common.rpc.http import HTTPClient
from sectools.common.utils.c_logging import logger
from sectools.features.isc.cfgparser.defines import CONFIG_STRUCTURE_GENERAL_PROPERTIES
from sectools.features.isc.signer.base_signer_v2 import BaseSignerV2
from sectools.features.isc.signer.signerutils.attributes import SigningAttributes


# These should be set to the production IP
QCOM_SIGNING_SERVER_HOST = 'http://localhost'
QCOM_SIGNING_SERVER_PORT = 50050
ENV_SIGNING_SERVER_HOST_TAG = 'SECTOOLS_SIGNER_HOST'
ENV_SIGNING_SERVER_PORT_TAG = 'SECTOOLS_SIGNER_PORT'


class QcomRemoteClient(HTTPClient):
    """Overrides the SecImageAPIIntf to allow one-shot signing of image files.
    """

    @HTTPClient.connect
    def sign_hash(self, request=None):
        pass


class QcomRemoteSigner(BaseSignerV2):

    SIGNING_ATTR_EXCLUDE = ['selected_signer',
                            'selected_encryptor',
                            'cass_capability',
                            'oem_id',
                            'selected_cert_config',
                            'object_id',
                            'debug_serials']

    @classmethod
    def is_plugin(cls):
        return True

    @classmethod
    def signer_id(cls):
        return 'qcom_remote'

    @staticmethod
    def get_general_properties_dict(general_properties):
        retval = {}
        for key in CONFIG_STRUCTURE_GENERAL_PROPERTIES:
            if key in QcomRemoteSigner.SIGNING_ATTR_EXCLUDE:
                continue
            retval[key] = getattr(general_properties, key)
        return retval

    def resolve_remote_server_info(self):
        host, port = QCOM_SIGNING_SERVER_HOST, QCOM_SIGNING_SERVER_PORT
        if ENV_SIGNING_SERVER_HOST_TAG in os.environ:
            logger.debug2(str(ENV_SIGNING_SERVER_HOST_TAG) + ' tag found in environment')
            host = os.environ[ENV_SIGNING_SERVER_HOST_TAG]
        if ENV_SIGNING_SERVER_PORT_TAG in os.environ:
            logger.debug2(str(ENV_SIGNING_SERVER_PORT_TAG) + ' tag found in environment')
            port = int(os.environ[ENV_SIGNING_SERVER_PORT_TAG])
        return host, port

    def initialize(self, imageinfo):
        """ The following should be set at the end of the call

        self.certs[BaseSignerV2.ROOT].cert
        self.certs[BaseSignerV2.CA].cert
        self.certs[BaseSignerV2.ATTEST].cert
        self.signature
        """
        # Initialize the base signer
        BaseSignerV2.initialize(self, imageinfo)

        # Create the request packet
        attrs = SigningAttributes()
        attrs.update_from_image_info_attrs(self.signing_attributes)
        request = {
            'machine': platform.node(),
            'sign_id': imageinfo.sign_id,
            'to_sign': BinString(self.hash_to_sign),
            'authority': imageinfo.authority,
            'signing_attributes': self.get_general_properties_dict(imageinfo.general_properties),
        }

        # Send the signing request
        remote_host, remote_port = self.resolve_remote_server_info()
        client = QcomRemoteClient(host=remote_host, port=remote_port)
        logger.info('Signing with the remote server at ' + remote_host + ':' +
                    str(remote_port) + '. Please wait for signing to complete.')
        response = client.sign_hash(request=request)

        # Check return code
        if response['retcode'] != 0:
            raise RuntimeError('Qcom remote signing failed [' + str(response['retcode']) + ']: ' +
                               str(response['errstr']))

        # Set the local vars
        root_cert, ca_cert, attest_cert, signature = (str(response['sig_pkg']['root']), str(response['sig_pkg']['ca']),
                                                      str(response['sig_pkg']['attest']),
                                                      str(response['sig_pkg']['signature']))

        # Make sure all assets are present
        if signature is None:
            raise RuntimeError("Signature is missing")
        if attest_cert is None:
            raise RuntimeError("Attestation Certificate is missing")
        if ca_cert is None:
            raise RuntimeError("CA Certificate is missing")
        if root_cert is None:
            raise RuntimeError("Root Certificate is missing")

        # Set all the variables
        self.certs[self.ROOT].cert = cert.get_cert_in_format(root_cert, utils.FORMAT_DER, utils.FORMAT_PEM)
        self.certs[self.CA].cert = cert.get_cert_in_format(ca_cert, utils.FORMAT_DER, utils.FORMAT_PEM)
        self.certs[self.ATTEST].cert = cert.get_cert_in_format(attest_cert, utils.FORMAT_DER, utils.FORMAT_PEM)
        self.signature = signature

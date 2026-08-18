# This container starts with Rocky9.6, installs Earthworm from the 
#   pre-compiled binaries at earthwormcentral.com, installs Python in a VENV,
#   and compiles PyEarthworm using the Earthworm ENV vars, CFLAGS, and includes.

ARG EW_HOME=/opt/earthworm
ARG EW_VERSION="ew_latest"
ARG EW_RUN_DIR=${EW_HOME}/run
ARG EW_LOG=${EW_RUN_DIR}/log
ARG EW_PARAMS=${EW_RUN_DIR}/params
ARG EW_DATA_DIR=${EW_RUN_DIR}/data
ARG EW_INSTALL=${EW_HOME}/${EW_VERSION}
ARG EW_BIN=${EW_HOME}/bin
ARG EW_INSTALL_INSTALLATION="INST_UNKNOWN"
ARG PYTHON_VER=3.11

FROM rockylinux/rockylinux:9.6-minimal

ARG EW_HOME
ARG EW_VERSION
ARG EW_RUN_DIR
ARG EW_LOG
ARG EW_PARAMS
ARG EW_DATA_DIR
ARG EW_INSTALL
ARG EW_BIN
ARG EW_INSTALL_INSTALLATION
ARG PYTHON_VER

# for use on USGS network (optional, fails gracefully)
RUN touch /etc/pki/ca-trust/source/anchors/ca-bundle.crt \
    && curl -s -o /etc/pki/ca-trust/source/anchors/ca-bundle.crt \
       https://apps-int.usgs.gov/ssl/DOIRootCA2.cer \
    && update-ca-trust || true

RUN microdnf -y install \
       gcc python${PYTHON_VER} python${PYTHON_VER}-devel python${PYTHON_VER}-pip \
       procps tar gzip wget hostname ca-certificates \
    && microdnf clean all

RUN mkdir -p "${EW_HOME}" "${EW_RUN_DIR}" "${EW_LOG}" "${EW_PARAMS}" \
             "${EW_DATA_DIR}" "${EW_BIN}" "${EW_HOME}/pyew"

# Download and extract pre-compiled Earthworm tgz
RUN wget -q http://earthwormcentral.com/distribution/earthworm_v8-0b8_rockylinux9_4.tar.gz \
       -O /tmp/earthworm.tar.gz \
    && tar -xzf /tmp/earthworm.tar.gz -C ${EW_HOME} \
    && mv ${EW_HOME}/earthworm_v8-0b8 ${EW_INSTALL} \
    && rm /tmp/earthworm.tar.gz

# copy setup file and ensure flags are set
RUN cp ${EW_INSTALL}/environment/ew_linux.bash ${EW_BIN}/ew_linux.bash \
    && sed -i 's|export GLOBALFLAGS="${CFLAGS} ${CPPFLAGS}"|export GLOBALFLAGS="${CFLAGS} ${CPPFLAGS} -m64 -fPIC"|' ${EW_BIN}/ew_linux.bash \
    && cp ${EW_INSTALL}/environment/earthworm_global.d ${EW_PARAMS}/earthworm_global.d

# Copy local bin scripts 
COPY test/earthworm/bin/ ${EW_BIN}/
RUN chmod +x ${EW_BIN}/*.sh ${EW_BIN}/start_ew 2>/dev/null || true

# Copy earthworm params and demo scripts
COPY test/earthworm/run/params/ ${EW_PARAMS}/
COPY test/earthworm/pyew/ ${EW_HOME}/pyew/

ENV EW_HOME=${EW_HOME}
ENV EW_VERSION=${EW_VERSION}
ENV EW_RUN_DIR=${EW_RUN_DIR}
ENV EW_LOG=${EW_LOG}
ENV EW_PARAMS=${EW_PARAMS}
ENV EW_DATA_DIR=${EW_DATA_DIR}
ENV EW_INSTALL=${EW_INSTALL}
ENV EW_BIN=${EW_BIN}
ENV EW_INSTALLATION=${EW_INSTALL_INSTALLATION}
ENV EW_INSTALL_VERSION=${EW_VERSION}
ENV EW_INSTALL_INSTALLATION=${EW_INSTALL_INSTALLATION}
ENV EWBITS=64
ENV PATH="${EW_BIN}:${PATH}"

# create venv
ENV VIRTUAL_ENV=${EW_HOME}/venv
RUN /usr/bin/python3.11 -m venv ${VIRTUAL_ENV}
ENV PATH="${VIRTUAL_ENV}/bin:${PATH}"

# copy this repo and install pyew
WORKDIR ${EW_HOME}/pyew
COPY . .
RUN rm -rf test
RUN /bin/bash -c "source ${EW_BIN}/ew_linux.bash \
    && pip install --upgrade pip setuptools wheel --timeout 120 --retries 5 \
    && pip install cython numpy --timeout 120 --retries 5 \
    && python setup.py install"

WORKDIR ${EW_HOME}
RUN echo "source ${EW_BIN}/ew_linux.bash" >> /root/.bashrc

ENTRYPOINT ["/opt/earthworm/bin/source_ew.sh"]
CMD ["startstop", ">>", "/opt/earthworm/run/log/startstop_log", "2>&1", "&"]
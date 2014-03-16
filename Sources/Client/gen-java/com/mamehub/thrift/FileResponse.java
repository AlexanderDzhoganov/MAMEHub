/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package com.mamehub.thrift;

import org.apache.thrift.scheme.IScheme;
import org.apache.thrift.scheme.SchemeFactory;
import org.apache.thrift.scheme.StandardScheme;

import org.apache.thrift.scheme.TupleScheme;
import org.apache.thrift.protocol.TTupleProtocol;
import org.apache.thrift.protocol.TProtocolException;
import org.apache.thrift.EncodingUtils;
import org.apache.thrift.TException;
import org.apache.thrift.async.AsyncMethodCallback;
import org.apache.thrift.server.AbstractNonblockingServer.*;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.EnumMap;
import java.util.Set;
import java.util.HashSet;
import java.util.EnumSet;
import java.util.Collections;
import java.util.BitSet;
import java.nio.ByteBuffer;
import java.util.Arrays;
import javax.annotation.Generated;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked"})
@Generated(value = "Autogenerated by Thrift Compiler (1.0.0-dev)", date = "2013-12-10")
public class FileResponse implements org.apache.thrift.TBase<FileResponse, FileResponse._Fields>, java.io.Serializable, Cloneable, Comparable<FileResponse> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("FileResponse");

  private static final org.apache.thrift.protocol.TField DATA_HEX_FIELD_DESC = new org.apache.thrift.protocol.TField("dataHex", org.apache.thrift.protocol.TType.STRING, (short)1);
  private static final org.apache.thrift.protocol.TField CODE_FIELD_DESC = new org.apache.thrift.protocol.TField("code", org.apache.thrift.protocol.TType.I32, (short)2);

  private static final Map<Class<? extends IScheme>, SchemeFactory> schemes = new HashMap<Class<? extends IScheme>, SchemeFactory>();
  static {
    schemes.put(StandardScheme.class, new FileResponseStandardSchemeFactory());
    schemes.put(TupleScheme.class, new FileResponseTupleSchemeFactory());
  }

  public String dataHex; // required
  /**
   * 
   * @see FileResponseCode
   */
  public FileResponseCode code; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    DATA_HEX((short)1, "dataHex"),
    /**
     * 
     * @see FileResponseCode
     */
    CODE((short)2, "code");

    private static final Map<String, _Fields> byName = new HashMap<String, _Fields>();

    static {
      for (_Fields field : EnumSet.allOf(_Fields.class)) {
        byName.put(field.getFieldName(), field);
      }
    }

    /**
     * Find the _Fields constant that matches fieldId, or null if its not found.
     */
    public static _Fields findByThriftId(int fieldId) {
      switch(fieldId) {
        case 1: // DATA_HEX
          return DATA_HEX;
        case 2: // CODE
          return CODE;
        default:
          return null;
      }
    }

    /**
     * Find the _Fields constant that matches fieldId, throwing an exception
     * if it is not found.
     */
    public static _Fields findByThriftIdOrThrow(int fieldId) {
      _Fields fields = findByThriftId(fieldId);
      if (fields == null) throw new IllegalArgumentException("Field " + fieldId + " doesn't exist!");
      return fields;
    }

    /**
     * Find the _Fields constant that matches name, or null if its not found.
     */
    public static _Fields findByName(String name) {
      return byName.get(name);
    }

    private final short _thriftId;
    private final String _fieldName;

    _Fields(short thriftId, String fieldName) {
      _thriftId = thriftId;
      _fieldName = fieldName;
    }

    public short getThriftFieldId() {
      return _thriftId;
    }

    public String getFieldName() {
      return _fieldName;
    }
  }

  // isset id assignments
  public static final Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.DATA_HEX, new org.apache.thrift.meta_data.FieldMetaData("dataHex", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.CODE, new org.apache.thrift.meta_data.FieldMetaData("code", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.EnumMetaData(org.apache.thrift.protocol.TType.ENUM, FileResponseCode.class)));
    metaDataMap = Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(FileResponse.class, metaDataMap);
  }

  public FileResponse() {
  }

  public FileResponse(
    String dataHex,
    FileResponseCode code)
  {
    this();
    this.dataHex = dataHex;
    this.code = code;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public FileResponse(FileResponse other) {
    if (other.isSetDataHex()) {
      this.dataHex = other.dataHex;
    }
    if (other.isSetCode()) {
      this.code = other.code;
    }
  }

  public FileResponse deepCopy() {
    return new FileResponse(this);
  }

  @Override
  public void clear() {
    this.dataHex = null;
    this.code = null;
  }

  public String getDataHex() {
    return this.dataHex;
  }

  public FileResponse setDataHex(String dataHex) {
    this.dataHex = dataHex;
    return this;
  }

  public void unsetDataHex() {
    this.dataHex = null;
  }

  /** Returns true if field dataHex is set (has been assigned a value) and false otherwise */
  public boolean isSetDataHex() {
    return this.dataHex != null;
  }

  public void setDataHexIsSet(boolean value) {
    if (!value) {
      this.dataHex = null;
    }
  }

  /**
   * 
   * @see FileResponseCode
   */
  public FileResponseCode getCode() {
    return this.code;
  }

  /**
   * 
   * @see FileResponseCode
   */
  public FileResponse setCode(FileResponseCode code) {
    this.code = code;
    return this;
  }

  public void unsetCode() {
    this.code = null;
  }

  /** Returns true if field code is set (has been assigned a value) and false otherwise */
  public boolean isSetCode() {
    return this.code != null;
  }

  public void setCodeIsSet(boolean value) {
    if (!value) {
      this.code = null;
    }
  }

  public void setFieldValue(_Fields field, Object value) {
    switch (field) {
    case DATA_HEX:
      if (value == null) {
        unsetDataHex();
      } else {
        setDataHex((String)value);
      }
      break;

    case CODE:
      if (value == null) {
        unsetCode();
      } else {
        setCode((FileResponseCode)value);
      }
      break;

    }
  }

  public Object getFieldValue(_Fields field) {
    switch (field) {
    case DATA_HEX:
      return getDataHex();

    case CODE:
      return getCode();

    }
    throw new IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new IllegalArgumentException();
    }

    switch (field) {
    case DATA_HEX:
      return isSetDataHex();
    case CODE:
      return isSetCode();
    }
    throw new IllegalStateException();
  }

  @Override
  public boolean equals(Object that) {
    if (that == null)
      return false;
    if (that instanceof FileResponse)
      return this.equals((FileResponse)that);
    return false;
  }

  public boolean equals(FileResponse that) {
    if (that == null)
      return false;

    boolean this_present_dataHex = true && this.isSetDataHex();
    boolean that_present_dataHex = true && that.isSetDataHex();
    if (this_present_dataHex || that_present_dataHex) {
      if (!(this_present_dataHex && that_present_dataHex))
        return false;
      if (!this.dataHex.equals(that.dataHex))
        return false;
    }

    boolean this_present_code = true && this.isSetCode();
    boolean that_present_code = true && that.isSetCode();
    if (this_present_code || that_present_code) {
      if (!(this_present_code && that_present_code))
        return false;
      if (!this.code.equals(that.code))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }

  @Override
  public int compareTo(FileResponse other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = Boolean.valueOf(isSetDataHex()).compareTo(other.isSetDataHex());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetDataHex()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.dataHex, other.dataHex);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetCode()).compareTo(other.isSetCode());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCode()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.code, other.code);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    return 0;
  }

  public _Fields fieldForId(int fieldId) {
    return _Fields.findByThriftId(fieldId);
  }

  public void read(org.apache.thrift.protocol.TProtocol iprot) throws org.apache.thrift.TException {
    schemes.get(iprot.getScheme()).getScheme().read(iprot, this);
  }

  public void write(org.apache.thrift.protocol.TProtocol oprot) throws org.apache.thrift.TException {
    schemes.get(oprot.getScheme()).getScheme().write(oprot, this);
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder("FileResponse(");
    boolean first = true;

    sb.append("dataHex:");
    if (this.dataHex == null) {
      sb.append("null");
    } else {
      sb.append(this.dataHex);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("code:");
    if (this.code == null) {
      sb.append("null");
    } else {
      sb.append(this.code);
    }
    first = false;
    sb.append(")");
    return sb.toString();
  }

  public void validate() throws org.apache.thrift.TException {
    // check for required fields
    // check for sub-struct validity
  }

  private void writeObject(java.io.ObjectOutputStream out) throws java.io.IOException {
    try {
      write(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(out)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private void readObject(java.io.ObjectInputStream in) throws java.io.IOException, ClassNotFoundException {
    try {
      read(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(in)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private static class FileResponseStandardSchemeFactory implements SchemeFactory {
    public FileResponseStandardScheme getScheme() {
      return new FileResponseStandardScheme();
    }
  }

  private static class FileResponseStandardScheme extends StandardScheme<FileResponse> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, FileResponse struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // DATA_HEX
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.dataHex = iprot.readString();
              struct.setDataHexIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // CODE
            if (schemeField.type == org.apache.thrift.protocol.TType.I32) {
              struct.code = FileResponseCode.findByValue(iprot.readI32());
              struct.setCodeIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          default:
            org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();

      // check for required fields of primitive type, which can't be checked in the validate method
      struct.validate();
    }

    public void write(org.apache.thrift.protocol.TProtocol oprot, FileResponse struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.dataHex != null) {
        oprot.writeFieldBegin(DATA_HEX_FIELD_DESC);
        oprot.writeString(struct.dataHex);
        oprot.writeFieldEnd();
      }
      if (struct.code != null) {
        oprot.writeFieldBegin(CODE_FIELD_DESC);
        oprot.writeI32(struct.code.getValue());
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class FileResponseTupleSchemeFactory implements SchemeFactory {
    public FileResponseTupleScheme getScheme() {
      return new FileResponseTupleScheme();
    }
  }

  private static class FileResponseTupleScheme extends TupleScheme<FileResponse> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, FileResponse struct) throws org.apache.thrift.TException {
      TTupleProtocol oprot = (TTupleProtocol) prot;
      BitSet optionals = new BitSet();
      if (struct.isSetDataHex()) {
        optionals.set(0);
      }
      if (struct.isSetCode()) {
        optionals.set(1);
      }
      oprot.writeBitSet(optionals, 2);
      if (struct.isSetDataHex()) {
        oprot.writeString(struct.dataHex);
      }
      if (struct.isSetCode()) {
        oprot.writeI32(struct.code.getValue());
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, FileResponse struct) throws org.apache.thrift.TException {
      TTupleProtocol iprot = (TTupleProtocol) prot;
      BitSet incoming = iprot.readBitSet(2);
      if (incoming.get(0)) {
        struct.dataHex = iprot.readString();
        struct.setDataHexIsSet(true);
      }
      if (incoming.get(1)) {
        struct.code = FileResponseCode.findByValue(iprot.readI32());
        struct.setCodeIsSet(true);
      }
    }
  }

}

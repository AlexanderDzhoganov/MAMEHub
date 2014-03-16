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
public class IpRangeData implements org.apache.thrift.TBase<IpRangeData, IpRangeData._Fields>, java.io.Serializable, Cloneable, Comparable<IpRangeData> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("IpRangeData");

  private static final org.apache.thrift.protocol.TField IP_START_FIELD_DESC = new org.apache.thrift.protocol.TField("ipStart", org.apache.thrift.protocol.TType.I64, (short)1);
  private static final org.apache.thrift.protocol.TField IP_END_FIELD_DESC = new org.apache.thrift.protocol.TField("ipEnd", org.apache.thrift.protocol.TType.I64, (short)2);
  private static final org.apache.thrift.protocol.TField COUNTRY_CODE2_FIELD_DESC = new org.apache.thrift.protocol.TField("countryCode2", org.apache.thrift.protocol.TType.STRING, (short)3);
  private static final org.apache.thrift.protocol.TField COUNTRY_CODE3_FIELD_DESC = new org.apache.thrift.protocol.TField("countryCode3", org.apache.thrift.protocol.TType.STRING, (short)4);
  private static final org.apache.thrift.protocol.TField COUNTRY_NAME_FIELD_DESC = new org.apache.thrift.protocol.TField("countryName", org.apache.thrift.protocol.TType.STRING, (short)5);

  private static final Map<Class<? extends IScheme>, SchemeFactory> schemes = new HashMap<Class<? extends IScheme>, SchemeFactory>();
  static {
    schemes.put(StandardScheme.class, new IpRangeDataStandardSchemeFactory());
    schemes.put(TupleScheme.class, new IpRangeDataTupleSchemeFactory());
  }

  public long ipStart; // required
  public long ipEnd; // required
  public String countryCode2; // required
  public String countryCode3; // required
  public String countryName; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    IP_START((short)1, "ipStart"),
    IP_END((short)2, "ipEnd"),
    COUNTRY_CODE2((short)3, "countryCode2"),
    COUNTRY_CODE3((short)4, "countryCode3"),
    COUNTRY_NAME((short)5, "countryName");

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
        case 1: // IP_START
          return IP_START;
        case 2: // IP_END
          return IP_END;
        case 3: // COUNTRY_CODE2
          return COUNTRY_CODE2;
        case 4: // COUNTRY_CODE3
          return COUNTRY_CODE3;
        case 5: // COUNTRY_NAME
          return COUNTRY_NAME;
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
  private static final int __IPSTART_ISSET_ID = 0;
  private static final int __IPEND_ISSET_ID = 1;
  private byte __isset_bitfield = 0;
  public static final Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.IP_START, new org.apache.thrift.meta_data.FieldMetaData("ipStart", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    tmpMap.put(_Fields.IP_END, new org.apache.thrift.meta_data.FieldMetaData("ipEnd", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    tmpMap.put(_Fields.COUNTRY_CODE2, new org.apache.thrift.meta_data.FieldMetaData("countryCode2", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.COUNTRY_CODE3, new org.apache.thrift.meta_data.FieldMetaData("countryCode3", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.COUNTRY_NAME, new org.apache.thrift.meta_data.FieldMetaData("countryName", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    metaDataMap = Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(IpRangeData.class, metaDataMap);
  }

  public IpRangeData() {
  }

  public IpRangeData(
    long ipStart,
    long ipEnd,
    String countryCode2,
    String countryCode3,
    String countryName)
  {
    this();
    this.ipStart = ipStart;
    setIpStartIsSet(true);
    this.ipEnd = ipEnd;
    setIpEndIsSet(true);
    this.countryCode2 = countryCode2;
    this.countryCode3 = countryCode3;
    this.countryName = countryName;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public IpRangeData(IpRangeData other) {
    __isset_bitfield = other.__isset_bitfield;
    this.ipStart = other.ipStart;
    this.ipEnd = other.ipEnd;
    if (other.isSetCountryCode2()) {
      this.countryCode2 = other.countryCode2;
    }
    if (other.isSetCountryCode3()) {
      this.countryCode3 = other.countryCode3;
    }
    if (other.isSetCountryName()) {
      this.countryName = other.countryName;
    }
  }

  public IpRangeData deepCopy() {
    return new IpRangeData(this);
  }

  @Override
  public void clear() {
    setIpStartIsSet(false);
    this.ipStart = 0;
    setIpEndIsSet(false);
    this.ipEnd = 0;
    this.countryCode2 = null;
    this.countryCode3 = null;
    this.countryName = null;
  }

  public long getIpStart() {
    return this.ipStart;
  }

  public IpRangeData setIpStart(long ipStart) {
    this.ipStart = ipStart;
    setIpStartIsSet(true);
    return this;
  }

  public void unsetIpStart() {
    __isset_bitfield = EncodingUtils.clearBit(__isset_bitfield, __IPSTART_ISSET_ID);
  }

  /** Returns true if field ipStart is set (has been assigned a value) and false otherwise */
  public boolean isSetIpStart() {
    return EncodingUtils.testBit(__isset_bitfield, __IPSTART_ISSET_ID);
  }

  public void setIpStartIsSet(boolean value) {
    __isset_bitfield = EncodingUtils.setBit(__isset_bitfield, __IPSTART_ISSET_ID, value);
  }

  public long getIpEnd() {
    return this.ipEnd;
  }

  public IpRangeData setIpEnd(long ipEnd) {
    this.ipEnd = ipEnd;
    setIpEndIsSet(true);
    return this;
  }

  public void unsetIpEnd() {
    __isset_bitfield = EncodingUtils.clearBit(__isset_bitfield, __IPEND_ISSET_ID);
  }

  /** Returns true if field ipEnd is set (has been assigned a value) and false otherwise */
  public boolean isSetIpEnd() {
    return EncodingUtils.testBit(__isset_bitfield, __IPEND_ISSET_ID);
  }

  public void setIpEndIsSet(boolean value) {
    __isset_bitfield = EncodingUtils.setBit(__isset_bitfield, __IPEND_ISSET_ID, value);
  }

  public String getCountryCode2() {
    return this.countryCode2;
  }

  public IpRangeData setCountryCode2(String countryCode2) {
    this.countryCode2 = countryCode2;
    return this;
  }

  public void unsetCountryCode2() {
    this.countryCode2 = null;
  }

  /** Returns true if field countryCode2 is set (has been assigned a value) and false otherwise */
  public boolean isSetCountryCode2() {
    return this.countryCode2 != null;
  }

  public void setCountryCode2IsSet(boolean value) {
    if (!value) {
      this.countryCode2 = null;
    }
  }

  public String getCountryCode3() {
    return this.countryCode3;
  }

  public IpRangeData setCountryCode3(String countryCode3) {
    this.countryCode3 = countryCode3;
    return this;
  }

  public void unsetCountryCode3() {
    this.countryCode3 = null;
  }

  /** Returns true if field countryCode3 is set (has been assigned a value) and false otherwise */
  public boolean isSetCountryCode3() {
    return this.countryCode3 != null;
  }

  public void setCountryCode3IsSet(boolean value) {
    if (!value) {
      this.countryCode3 = null;
    }
  }

  public String getCountryName() {
    return this.countryName;
  }

  public IpRangeData setCountryName(String countryName) {
    this.countryName = countryName;
    return this;
  }

  public void unsetCountryName() {
    this.countryName = null;
  }

  /** Returns true if field countryName is set (has been assigned a value) and false otherwise */
  public boolean isSetCountryName() {
    return this.countryName != null;
  }

  public void setCountryNameIsSet(boolean value) {
    if (!value) {
      this.countryName = null;
    }
  }

  public void setFieldValue(_Fields field, Object value) {
    switch (field) {
    case IP_START:
      if (value == null) {
        unsetIpStart();
      } else {
        setIpStart((Long)value);
      }
      break;

    case IP_END:
      if (value == null) {
        unsetIpEnd();
      } else {
        setIpEnd((Long)value);
      }
      break;

    case COUNTRY_CODE2:
      if (value == null) {
        unsetCountryCode2();
      } else {
        setCountryCode2((String)value);
      }
      break;

    case COUNTRY_CODE3:
      if (value == null) {
        unsetCountryCode3();
      } else {
        setCountryCode3((String)value);
      }
      break;

    case COUNTRY_NAME:
      if (value == null) {
        unsetCountryName();
      } else {
        setCountryName((String)value);
      }
      break;

    }
  }

  public Object getFieldValue(_Fields field) {
    switch (field) {
    case IP_START:
      return Long.valueOf(getIpStart());

    case IP_END:
      return Long.valueOf(getIpEnd());

    case COUNTRY_CODE2:
      return getCountryCode2();

    case COUNTRY_CODE3:
      return getCountryCode3();

    case COUNTRY_NAME:
      return getCountryName();

    }
    throw new IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new IllegalArgumentException();
    }

    switch (field) {
    case IP_START:
      return isSetIpStart();
    case IP_END:
      return isSetIpEnd();
    case COUNTRY_CODE2:
      return isSetCountryCode2();
    case COUNTRY_CODE3:
      return isSetCountryCode3();
    case COUNTRY_NAME:
      return isSetCountryName();
    }
    throw new IllegalStateException();
  }

  @Override
  public boolean equals(Object that) {
    if (that == null)
      return false;
    if (that instanceof IpRangeData)
      return this.equals((IpRangeData)that);
    return false;
  }

  public boolean equals(IpRangeData that) {
    if (that == null)
      return false;

    boolean this_present_ipStart = true;
    boolean that_present_ipStart = true;
    if (this_present_ipStart || that_present_ipStart) {
      if (!(this_present_ipStart && that_present_ipStart))
        return false;
      if (this.ipStart != that.ipStart)
        return false;
    }

    boolean this_present_ipEnd = true;
    boolean that_present_ipEnd = true;
    if (this_present_ipEnd || that_present_ipEnd) {
      if (!(this_present_ipEnd && that_present_ipEnd))
        return false;
      if (this.ipEnd != that.ipEnd)
        return false;
    }

    boolean this_present_countryCode2 = true && this.isSetCountryCode2();
    boolean that_present_countryCode2 = true && that.isSetCountryCode2();
    if (this_present_countryCode2 || that_present_countryCode2) {
      if (!(this_present_countryCode2 && that_present_countryCode2))
        return false;
      if (!this.countryCode2.equals(that.countryCode2))
        return false;
    }

    boolean this_present_countryCode3 = true && this.isSetCountryCode3();
    boolean that_present_countryCode3 = true && that.isSetCountryCode3();
    if (this_present_countryCode3 || that_present_countryCode3) {
      if (!(this_present_countryCode3 && that_present_countryCode3))
        return false;
      if (!this.countryCode3.equals(that.countryCode3))
        return false;
    }

    boolean this_present_countryName = true && this.isSetCountryName();
    boolean that_present_countryName = true && that.isSetCountryName();
    if (this_present_countryName || that_present_countryName) {
      if (!(this_present_countryName && that_present_countryName))
        return false;
      if (!this.countryName.equals(that.countryName))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }

  @Override
  public int compareTo(IpRangeData other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = Boolean.valueOf(isSetIpStart()).compareTo(other.isSetIpStart());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetIpStart()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.ipStart, other.ipStart);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetIpEnd()).compareTo(other.isSetIpEnd());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetIpEnd()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.ipEnd, other.ipEnd);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetCountryCode2()).compareTo(other.isSetCountryCode2());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCountryCode2()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.countryCode2, other.countryCode2);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetCountryCode3()).compareTo(other.isSetCountryCode3());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCountryCode3()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.countryCode3, other.countryCode3);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetCountryName()).compareTo(other.isSetCountryName());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCountryName()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.countryName, other.countryName);
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
    StringBuilder sb = new StringBuilder("IpRangeData(");
    boolean first = true;

    sb.append("ipStart:");
    sb.append(this.ipStart);
    first = false;
    if (!first) sb.append(", ");
    sb.append("ipEnd:");
    sb.append(this.ipEnd);
    first = false;
    if (!first) sb.append(", ");
    sb.append("countryCode2:");
    if (this.countryCode2 == null) {
      sb.append("null");
    } else {
      sb.append(this.countryCode2);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("countryCode3:");
    if (this.countryCode3 == null) {
      sb.append("null");
    } else {
      sb.append(this.countryCode3);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("countryName:");
    if (this.countryName == null) {
      sb.append("null");
    } else {
      sb.append(this.countryName);
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
      // it doesn't seem like you should have to do this, but java serialization is wacky, and doesn't call the default constructor.
      __isset_bitfield = 0;
      read(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(in)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private static class IpRangeDataStandardSchemeFactory implements SchemeFactory {
    public IpRangeDataStandardScheme getScheme() {
      return new IpRangeDataStandardScheme();
    }
  }

  private static class IpRangeDataStandardScheme extends StandardScheme<IpRangeData> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, IpRangeData struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // IP_START
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.ipStart = iprot.readI64();
              struct.setIpStartIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // IP_END
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.ipEnd = iprot.readI64();
              struct.setIpEndIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 3: // COUNTRY_CODE2
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.countryCode2 = iprot.readString();
              struct.setCountryCode2IsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 4: // COUNTRY_CODE3
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.countryCode3 = iprot.readString();
              struct.setCountryCode3IsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 5: // COUNTRY_NAME
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.countryName = iprot.readString();
              struct.setCountryNameIsSet(true);
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

    public void write(org.apache.thrift.protocol.TProtocol oprot, IpRangeData struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      oprot.writeFieldBegin(IP_START_FIELD_DESC);
      oprot.writeI64(struct.ipStart);
      oprot.writeFieldEnd();
      oprot.writeFieldBegin(IP_END_FIELD_DESC);
      oprot.writeI64(struct.ipEnd);
      oprot.writeFieldEnd();
      if (struct.countryCode2 != null) {
        oprot.writeFieldBegin(COUNTRY_CODE2_FIELD_DESC);
        oprot.writeString(struct.countryCode2);
        oprot.writeFieldEnd();
      }
      if (struct.countryCode3 != null) {
        oprot.writeFieldBegin(COUNTRY_CODE3_FIELD_DESC);
        oprot.writeString(struct.countryCode3);
        oprot.writeFieldEnd();
      }
      if (struct.countryName != null) {
        oprot.writeFieldBegin(COUNTRY_NAME_FIELD_DESC);
        oprot.writeString(struct.countryName);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class IpRangeDataTupleSchemeFactory implements SchemeFactory {
    public IpRangeDataTupleScheme getScheme() {
      return new IpRangeDataTupleScheme();
    }
  }

  private static class IpRangeDataTupleScheme extends TupleScheme<IpRangeData> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, IpRangeData struct) throws org.apache.thrift.TException {
      TTupleProtocol oprot = (TTupleProtocol) prot;
      BitSet optionals = new BitSet();
      if (struct.isSetIpStart()) {
        optionals.set(0);
      }
      if (struct.isSetIpEnd()) {
        optionals.set(1);
      }
      if (struct.isSetCountryCode2()) {
        optionals.set(2);
      }
      if (struct.isSetCountryCode3()) {
        optionals.set(3);
      }
      if (struct.isSetCountryName()) {
        optionals.set(4);
      }
      oprot.writeBitSet(optionals, 5);
      if (struct.isSetIpStart()) {
        oprot.writeI64(struct.ipStart);
      }
      if (struct.isSetIpEnd()) {
        oprot.writeI64(struct.ipEnd);
      }
      if (struct.isSetCountryCode2()) {
        oprot.writeString(struct.countryCode2);
      }
      if (struct.isSetCountryCode3()) {
        oprot.writeString(struct.countryCode3);
      }
      if (struct.isSetCountryName()) {
        oprot.writeString(struct.countryName);
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, IpRangeData struct) throws org.apache.thrift.TException {
      TTupleProtocol iprot = (TTupleProtocol) prot;
      BitSet incoming = iprot.readBitSet(5);
      if (incoming.get(0)) {
        struct.ipStart = iprot.readI64();
        struct.setIpStartIsSet(true);
      }
      if (incoming.get(1)) {
        struct.ipEnd = iprot.readI64();
        struct.setIpEndIsSet(true);
      }
      if (incoming.get(2)) {
        struct.countryCode2 = iprot.readString();
        struct.setCountryCode2IsSet(true);
      }
      if (incoming.get(3)) {
        struct.countryCode3 = iprot.readString();
        struct.setCountryCode3IsSet(true);
      }
      if (incoming.get(4)) {
        struct.countryName = iprot.readString();
        struct.setCountryNameIsSet(true);
      }
    }
  }

}

/**
 * Autogenerated by Thrift Compiler (0.9.0-dev)
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
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class FileInfo implements org.apache.thrift.TBase<FileInfo, FileInfo._Fields>, java.io.Serializable, Cloneable {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("FileInfo");

  private static final org.apache.thrift.protocol.TField ID_FIELD_DESC = new org.apache.thrift.protocol.TField("id", org.apache.thrift.protocol.TType.STRING, (short)1);
  private static final org.apache.thrift.protocol.TField BAD_FIELD_DESC = new org.apache.thrift.protocol.TField("bad", org.apache.thrift.protocol.TType.BOOL, (short)2);
  private static final org.apache.thrift.protocol.TField CRC32_FIELD_DESC = new org.apache.thrift.protocol.TField("crc32", org.apache.thrift.protocol.TType.STRING, (short)3);
  private static final org.apache.thrift.protocol.TField LENGTH_FIELD_DESC = new org.apache.thrift.protocol.TField("length", org.apache.thrift.protocol.TType.I64, (short)4);
  private static final org.apache.thrift.protocol.TField CONTENTS_CRC32_FIELD_DESC = new org.apache.thrift.protocol.TField("contentsCrc32", org.apache.thrift.protocol.TType.MAP, (short)5);
  private static final org.apache.thrift.protocol.TField CHDNAME_FIELD_DESC = new org.apache.thrift.protocol.TField("chdname", org.apache.thrift.protocol.TType.STRING, (short)6);

  private static final Map<Class<? extends IScheme>, SchemeFactory> schemes = new HashMap<Class<? extends IScheme>, SchemeFactory>();
  static {
    schemes.put(StandardScheme.class, new FileInfoStandardSchemeFactory());
    schemes.put(TupleScheme.class, new FileInfoTupleSchemeFactory());
  }

  public String id; // required
  public boolean bad; // required
  public String crc32; // required
  public long length; // required
  public Map<String,String> contentsCrc32; // required
  public String chdname; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    ID((short)1, "id"),
    BAD((short)2, "bad"),
    CRC32((short)3, "crc32"),
    LENGTH((short)4, "length"),
    CONTENTS_CRC32((short)5, "contentsCrc32"),
    CHDNAME((short)6, "chdname");

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
        case 1: // ID
          return ID;
        case 2: // BAD
          return BAD;
        case 3: // CRC32
          return CRC32;
        case 4: // LENGTH
          return LENGTH;
        case 5: // CONTENTS_CRC32
          return CONTENTS_CRC32;
        case 6: // CHDNAME
          return CHDNAME;
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
  private static final int __BAD_ISSET_ID = 0;
  private static final int __LENGTH_ISSET_ID = 1;
  private byte __isset_bitfield = 0;
  public static final Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.ID, new org.apache.thrift.meta_data.FieldMetaData("id", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.BAD, new org.apache.thrift.meta_data.FieldMetaData("bad", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.BOOL)));
    tmpMap.put(_Fields.CRC32, new org.apache.thrift.meta_data.FieldMetaData("crc32", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.LENGTH, new org.apache.thrift.meta_data.FieldMetaData("length", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    tmpMap.put(_Fields.CONTENTS_CRC32, new org.apache.thrift.meta_data.FieldMetaData("contentsCrc32", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.MapMetaData(org.apache.thrift.protocol.TType.MAP, 
            new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING), 
            new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING))));
    tmpMap.put(_Fields.CHDNAME, new org.apache.thrift.meta_data.FieldMetaData("chdname", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    metaDataMap = Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(FileInfo.class, metaDataMap);
  }

  public FileInfo() {
    this.contentsCrc32 = new HashMap<String,String>();

  }

  public FileInfo(
    String id,
    boolean bad,
    String crc32,
    long length,
    Map<String,String> contentsCrc32,
    String chdname)
  {
    this();
    this.id = id;
    this.bad = bad;
    setBadIsSet(true);
    this.crc32 = crc32;
    this.length = length;
    setLengthIsSet(true);
    this.contentsCrc32 = contentsCrc32;
    this.chdname = chdname;
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public FileInfo(FileInfo other) {
    __isset_bitfield = other.__isset_bitfield;
    if (other.isSetId()) {
      this.id = other.id;
    }
    this.bad = other.bad;
    if (other.isSetCrc32()) {
      this.crc32 = other.crc32;
    }
    this.length = other.length;
    if (other.isSetContentsCrc32()) {
      Map<String,String> __this__contentsCrc32 = new HashMap<String,String>();
      for (Map.Entry<String, String> other_element : other.contentsCrc32.entrySet()) {

        String other_element_key = other_element.getKey();
        String other_element_value = other_element.getValue();

        String __this__contentsCrc32_copy_key = other_element_key;

        String __this__contentsCrc32_copy_value = other_element_value;

        __this__contentsCrc32.put(__this__contentsCrc32_copy_key, __this__contentsCrc32_copy_value);
      }
      this.contentsCrc32 = __this__contentsCrc32;
    }
    if (other.isSetChdname()) {
      this.chdname = other.chdname;
    }
  }

  public FileInfo deepCopy() {
    return new FileInfo(this);
  }

  @Override
  public void clear() {
    this.id = null;
    setBadIsSet(false);
    this.bad = false;
    this.crc32 = null;
    setLengthIsSet(false);
    this.length = 0;
    this.contentsCrc32 = new HashMap<String,String>();

    this.chdname = null;
  }

  public String getId() {
    return this.id;
  }

  public FileInfo setId(String id) {
    this.id = id;
    return this;
  }

  public void unsetId() {
    this.id = null;
  }

  /** Returns true if field id is set (has been assigned a value) and false otherwise */
  public boolean isSetId() {
    return this.id != null;
  }

  public void setIdIsSet(boolean value) {
    if (!value) {
      this.id = null;
    }
  }

  public boolean isBad() {
    return this.bad;
  }

  public FileInfo setBad(boolean bad) {
    this.bad = bad;
    setBadIsSet(true);
    return this;
  }

  public void unsetBad() {
    __isset_bitfield = EncodingUtils.clearBit(__isset_bitfield, __BAD_ISSET_ID);
  }

  /** Returns true if field bad is set (has been assigned a value) and false otherwise */
  public boolean isSetBad() {
    return EncodingUtils.testBit(__isset_bitfield, __BAD_ISSET_ID);
  }

  public void setBadIsSet(boolean value) {
    __isset_bitfield = EncodingUtils.setBit(__isset_bitfield, __BAD_ISSET_ID, value);
  }

  public String getCrc32() {
    return this.crc32;
  }

  public FileInfo setCrc32(String crc32) {
    this.crc32 = crc32;
    return this;
  }

  public void unsetCrc32() {
    this.crc32 = null;
  }

  /** Returns true if field crc32 is set (has been assigned a value) and false otherwise */
  public boolean isSetCrc32() {
    return this.crc32 != null;
  }

  public void setCrc32IsSet(boolean value) {
    if (!value) {
      this.crc32 = null;
    }
  }

  public long getLength() {
    return this.length;
  }

  public FileInfo setLength(long length) {
    this.length = length;
    setLengthIsSet(true);
    return this;
  }

  public void unsetLength() {
    __isset_bitfield = EncodingUtils.clearBit(__isset_bitfield, __LENGTH_ISSET_ID);
  }

  /** Returns true if field length is set (has been assigned a value) and false otherwise */
  public boolean isSetLength() {
    return EncodingUtils.testBit(__isset_bitfield, __LENGTH_ISSET_ID);
  }

  public void setLengthIsSet(boolean value) {
    __isset_bitfield = EncodingUtils.setBit(__isset_bitfield, __LENGTH_ISSET_ID, value);
  }

  public int getContentsCrc32Size() {
    return (this.contentsCrc32 == null) ? 0 : this.contentsCrc32.size();
  }

  public void putToContentsCrc32(String key, String val) {
    if (this.contentsCrc32 == null) {
      this.contentsCrc32 = new HashMap<String,String>();
    }
    this.contentsCrc32.put(key, val);
  }

  public Map<String,String> getContentsCrc32() {
    return this.contentsCrc32;
  }

  public FileInfo setContentsCrc32(Map<String,String> contentsCrc32) {
    this.contentsCrc32 = contentsCrc32;
    return this;
  }

  public void unsetContentsCrc32() {
    this.contentsCrc32 = null;
  }

  /** Returns true if field contentsCrc32 is set (has been assigned a value) and false otherwise */
  public boolean isSetContentsCrc32() {
    return this.contentsCrc32 != null;
  }

  public void setContentsCrc32IsSet(boolean value) {
    if (!value) {
      this.contentsCrc32 = null;
    }
  }

  public String getChdname() {
    return this.chdname;
  }

  public FileInfo setChdname(String chdname) {
    this.chdname = chdname;
    return this;
  }

  public void unsetChdname() {
    this.chdname = null;
  }

  /** Returns true if field chdname is set (has been assigned a value) and false otherwise */
  public boolean isSetChdname() {
    return this.chdname != null;
  }

  public void setChdnameIsSet(boolean value) {
    if (!value) {
      this.chdname = null;
    }
  }

  public void setFieldValue(_Fields field, Object value) {
    switch (field) {
    case ID:
      if (value == null) {
        unsetId();
      } else {
        setId((String)value);
      }
      break;

    case BAD:
      if (value == null) {
        unsetBad();
      } else {
        setBad((Boolean)value);
      }
      break;

    case CRC32:
      if (value == null) {
        unsetCrc32();
      } else {
        setCrc32((String)value);
      }
      break;

    case LENGTH:
      if (value == null) {
        unsetLength();
      } else {
        setLength((Long)value);
      }
      break;

    case CONTENTS_CRC32:
      if (value == null) {
        unsetContentsCrc32();
      } else {
        setContentsCrc32((Map<String,String>)value);
      }
      break;

    case CHDNAME:
      if (value == null) {
        unsetChdname();
      } else {
        setChdname((String)value);
      }
      break;

    }
  }

  public Object getFieldValue(_Fields field) {
    switch (field) {
    case ID:
      return getId();

    case BAD:
      return Boolean.valueOf(isBad());

    case CRC32:
      return getCrc32();

    case LENGTH:
      return Long.valueOf(getLength());

    case CONTENTS_CRC32:
      return getContentsCrc32();

    case CHDNAME:
      return getChdname();

    }
    throw new IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new IllegalArgumentException();
    }

    switch (field) {
    case ID:
      return isSetId();
    case BAD:
      return isSetBad();
    case CRC32:
      return isSetCrc32();
    case LENGTH:
      return isSetLength();
    case CONTENTS_CRC32:
      return isSetContentsCrc32();
    case CHDNAME:
      return isSetChdname();
    }
    throw new IllegalStateException();
  }

  @Override
  public boolean equals(Object that) {
    if (that == null)
      return false;
    if (that instanceof FileInfo)
      return this.equals((FileInfo)that);
    return false;
  }

  public boolean equals(FileInfo that) {
    if (that == null)
      return false;

    boolean this_present_id = true && this.isSetId();
    boolean that_present_id = true && that.isSetId();
    if (this_present_id || that_present_id) {
      if (!(this_present_id && that_present_id))
        return false;
      if (!this.id.equals(that.id))
        return false;
    }

    boolean this_present_bad = true;
    boolean that_present_bad = true;
    if (this_present_bad || that_present_bad) {
      if (!(this_present_bad && that_present_bad))
        return false;
      if (this.bad != that.bad)
        return false;
    }

    boolean this_present_crc32 = true && this.isSetCrc32();
    boolean that_present_crc32 = true && that.isSetCrc32();
    if (this_present_crc32 || that_present_crc32) {
      if (!(this_present_crc32 && that_present_crc32))
        return false;
      if (!this.crc32.equals(that.crc32))
        return false;
    }

    boolean this_present_length = true;
    boolean that_present_length = true;
    if (this_present_length || that_present_length) {
      if (!(this_present_length && that_present_length))
        return false;
      if (this.length != that.length)
        return false;
    }

    boolean this_present_contentsCrc32 = true && this.isSetContentsCrc32();
    boolean that_present_contentsCrc32 = true && that.isSetContentsCrc32();
    if (this_present_contentsCrc32 || that_present_contentsCrc32) {
      if (!(this_present_contentsCrc32 && that_present_contentsCrc32))
        return false;
      if (!this.contentsCrc32.equals(that.contentsCrc32))
        return false;
    }

    boolean this_present_chdname = true && this.isSetChdname();
    boolean that_present_chdname = true && that.isSetChdname();
    if (this_present_chdname || that_present_chdname) {
      if (!(this_present_chdname && that_present_chdname))
        return false;
      if (!this.chdname.equals(that.chdname))
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    return 0;
  }

  public int compareTo(FileInfo other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;
    FileInfo typedOther = (FileInfo)other;

    lastComparison = Boolean.valueOf(isSetId()).compareTo(typedOther.isSetId());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetId()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.id, typedOther.id);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetBad()).compareTo(typedOther.isSetBad());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetBad()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.bad, typedOther.bad);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetCrc32()).compareTo(typedOther.isSetCrc32());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetCrc32()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.crc32, typedOther.crc32);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetLength()).compareTo(typedOther.isSetLength());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetLength()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.length, typedOther.length);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetContentsCrc32()).compareTo(typedOther.isSetContentsCrc32());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetContentsCrc32()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.contentsCrc32, typedOther.contentsCrc32);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = Boolean.valueOf(isSetChdname()).compareTo(typedOther.isSetChdname());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetChdname()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.chdname, typedOther.chdname);
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
    StringBuilder sb = new StringBuilder("FileInfo(");
    boolean first = true;

    sb.append("id:");
    if (this.id == null) {
      sb.append("null");
    } else {
      sb.append(this.id);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("bad:");
    sb.append(this.bad);
    first = false;
    if (!first) sb.append(", ");
    sb.append("crc32:");
    if (this.crc32 == null) {
      sb.append("null");
    } else {
      sb.append(this.crc32);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("length:");
    sb.append(this.length);
    first = false;
    if (!first) sb.append(", ");
    sb.append("contentsCrc32:");
    if (this.contentsCrc32 == null) {
      sb.append("null");
    } else {
      sb.append(this.contentsCrc32);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("chdname:");
    if (this.chdname == null) {
      sb.append("null");
    } else {
      sb.append(this.chdname);
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

  private static class FileInfoStandardSchemeFactory implements SchemeFactory {
    public FileInfoStandardScheme getScheme() {
      return new FileInfoStandardScheme();
    }
  }

  private static class FileInfoStandardScheme extends StandardScheme<FileInfo> {

    public void read(org.apache.thrift.protocol.TProtocol iprot, FileInfo struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // ID
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.id = iprot.readString();
              struct.setIdIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // BAD
            if (schemeField.type == org.apache.thrift.protocol.TType.BOOL) {
              struct.bad = iprot.readBool();
              struct.setBadIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 3: // CRC32
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.crc32 = iprot.readString();
              struct.setCrc32IsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 4: // LENGTH
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.length = iprot.readI64();
              struct.setLengthIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 5: // CONTENTS_CRC32
            if (schemeField.type == org.apache.thrift.protocol.TType.MAP) {
              {
                org.apache.thrift.protocol.TMap _map8 = iprot.readMapBegin();
                struct.contentsCrc32 = new HashMap<String,String>(2*_map8.size);
                for (int _i9 = 0; _i9 < _map8.size; ++_i9)
                {
                  String _key10; // required
                  String _val11; // required
                  _key10 = iprot.readString();
                  _val11 = iprot.readString();
                  struct.contentsCrc32.put(_key10, _val11);
                }
                iprot.readMapEnd();
              }
              struct.setContentsCrc32IsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 6: // CHDNAME
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.chdname = iprot.readString();
              struct.setChdnameIsSet(true);
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

    public void write(org.apache.thrift.protocol.TProtocol oprot, FileInfo struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.id != null) {
        oprot.writeFieldBegin(ID_FIELD_DESC);
        oprot.writeString(struct.id);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldBegin(BAD_FIELD_DESC);
      oprot.writeBool(struct.bad);
      oprot.writeFieldEnd();
      if (struct.crc32 != null) {
        oprot.writeFieldBegin(CRC32_FIELD_DESC);
        oprot.writeString(struct.crc32);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldBegin(LENGTH_FIELD_DESC);
      oprot.writeI64(struct.length);
      oprot.writeFieldEnd();
      if (struct.contentsCrc32 != null) {
        oprot.writeFieldBegin(CONTENTS_CRC32_FIELD_DESC);
        {
          oprot.writeMapBegin(new org.apache.thrift.protocol.TMap(org.apache.thrift.protocol.TType.STRING, org.apache.thrift.protocol.TType.STRING, struct.contentsCrc32.size()));
          for (Map.Entry<String, String> _iter12 : struct.contentsCrc32.entrySet())
          {
            oprot.writeString(_iter12.getKey());
            oprot.writeString(_iter12.getValue());
          }
          oprot.writeMapEnd();
        }
        oprot.writeFieldEnd();
      }
      if (struct.chdname != null) {
        oprot.writeFieldBegin(CHDNAME_FIELD_DESC);
        oprot.writeString(struct.chdname);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class FileInfoTupleSchemeFactory implements SchemeFactory {
    public FileInfoTupleScheme getScheme() {
      return new FileInfoTupleScheme();
    }
  }

  private static class FileInfoTupleScheme extends TupleScheme<FileInfo> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, FileInfo struct) throws org.apache.thrift.TException {
      TTupleProtocol oprot = (TTupleProtocol) prot;
      BitSet optionals = new BitSet();
      if (struct.isSetId()) {
        optionals.set(0);
      }
      if (struct.isSetBad()) {
        optionals.set(1);
      }
      if (struct.isSetCrc32()) {
        optionals.set(2);
      }
      if (struct.isSetLength()) {
        optionals.set(3);
      }
      if (struct.isSetContentsCrc32()) {
        optionals.set(4);
      }
      if (struct.isSetChdname()) {
        optionals.set(5);
      }
      oprot.writeBitSet(optionals, 6);
      if (struct.isSetId()) {
        oprot.writeString(struct.id);
      }
      if (struct.isSetBad()) {
        oprot.writeBool(struct.bad);
      }
      if (struct.isSetCrc32()) {
        oprot.writeString(struct.crc32);
      }
      if (struct.isSetLength()) {
        oprot.writeI64(struct.length);
      }
      if (struct.isSetContentsCrc32()) {
        {
          oprot.writeI32(struct.contentsCrc32.size());
          for (Map.Entry<String, String> _iter13 : struct.contentsCrc32.entrySet())
          {
            oprot.writeString(_iter13.getKey());
            oprot.writeString(_iter13.getValue());
          }
        }
      }
      if (struct.isSetChdname()) {
        oprot.writeString(struct.chdname);
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, FileInfo struct) throws org.apache.thrift.TException {
      TTupleProtocol iprot = (TTupleProtocol) prot;
      BitSet incoming = iprot.readBitSet(6);
      if (incoming.get(0)) {
        struct.id = iprot.readString();
        struct.setIdIsSet(true);
      }
      if (incoming.get(1)) {
        struct.bad = iprot.readBool();
        struct.setBadIsSet(true);
      }
      if (incoming.get(2)) {
        struct.crc32 = iprot.readString();
        struct.setCrc32IsSet(true);
      }
      if (incoming.get(3)) {
        struct.length = iprot.readI64();
        struct.setLengthIsSet(true);
      }
      if (incoming.get(4)) {
        {
          org.apache.thrift.protocol.TMap _map14 = new org.apache.thrift.protocol.TMap(org.apache.thrift.protocol.TType.STRING, org.apache.thrift.protocol.TType.STRING, iprot.readI32());
          struct.contentsCrc32 = new HashMap<String,String>(2*_map14.size);
          for (int _i15 = 0; _i15 < _map14.size; ++_i15)
          {
            String _key16; // required
            String _val17; // required
            _key16 = iprot.readString();
            _val17 = iprot.readString();
            struct.contentsCrc32.put(_key16, _val17);
          }
        }
        struct.setContentsCrc32IsSet(true);
      }
      if (incoming.get(5)) {
        struct.chdname = iprot.readString();
        struct.setChdnameIsSet(true);
      }
    }
  }

}


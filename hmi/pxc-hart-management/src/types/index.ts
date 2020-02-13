export interface HartDeviceDto {
    name: string
    company: string
    longTag: string
    cmdRevLvl: number
    manufacturerId: number
    privateLabelDistCode: number
    profile: number
    revision: number
    ioCard: number
    channel: number
    vars?: HartVars
}

export const verboseHartDeviceAttrs = {
    // name: "Name",
    // company: "Company",
    // longTag: "Long Tag",
    cmdRevLvl: "HART Revision",
    revision: "Device Revision",
    ioCard: "I/O Card",
    channel: "Channel"
}

export interface HartMuxDto extends HartDeviceDto {
    devices: HartDeviceDto[]
    ipAddress: string
}

export interface HartGw {
    ip: string
    modules: string[]
    serialNo: number
}

export interface HartGwDto {
    gateways: HartGw[]
}

export interface HartVarsDto {
    loopCurrent: {
        units: string
        value: number
    }
    pv: {
        units: string
        value: number
    }
    sv: {
        units: string
        value: number
    }
    tv: {
        units: string
        value: number
    }
    qv: {
        units: string
        value: number
    }
}

export interface HartVars {
    loopCurrent: {
        value: number,
        units: string,
        lastUpdated?: Date
    }
    pv: {
        value: number,
        units: string,
        lastUpdated?: Date
    }
    sv?: {
        value: number,
        units: string,
        lastUpdated?: Date
    }
    tv?: {
        value: number,
        units: string,
        lastUpdated?: Date
    }
    qv?: {
        value: number,
        units: string,
        lastUpdated?: Date
    }
}

export interface HartDeviceVarsListDto {
    devices: HartDeviceDto[]
}

export const hartServerUrl = "http://localhost:5900"
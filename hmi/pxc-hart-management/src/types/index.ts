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
}

export interface HartMuxDto extends HartDeviceDto {
    devices: HartDeviceDto[]
    ipAddress: string
}

export interface HartGw {
    ip: string
    modules: string[],
    serialNo: number
}

export interface HartGwDto {
    gateways: HartGw[]
}

export const hartServerUrl = "http://localhost:5900"
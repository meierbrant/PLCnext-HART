export interface HartDeviceDto {
    name: string
    company: string
    longTag: string
    cmdRevLvl: number
    manufacturerId: number
    privateLabelDistCode: number
    profile: number
    revision: number,
    ioCard: number,
    channel: number
}

export interface HartMuxDto extends HartDeviceDto {
    devices: HartDeviceDto[]
}

export const hartServerUrl = "http://localhost:5900"